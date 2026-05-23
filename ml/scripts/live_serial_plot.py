import argparse
import re
import sys
import time
from collections import deque

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import serial


FIELDS = {
    "Sound Level": "sound_level",
    "Peak Frequency": "peak_frequency",
    "Spectral Center": "spectral_center",
    "Zero Cross Rate": "zero_cross_rate",
    "Leak Probability": "leak_probability",
    "Audio Gate": "audio_gate",
    "Model Gate": "model_gate",
    "Vibration Gate": "vibration_gate",
    "Candidate Leak": "candidate_leak",
    "Leak Count": "leak_count",
    "Final Result": "final_result",
    "LED": "led",
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Live plot ESP32 leak model values from Serial output."
    )
    parser.add_argument("--port", required=True, help="Serial port, for example COM3")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument(
        "--history",
        type=int,
        default=120,
        help="How many samples to keep in the rolling plot",
    )
    return parser.parse_args()


def parse_value(label, raw_value):
    raw_value = raw_value.strip()
    if label in {"Audio Gate", "Model Gate", "Vibration Gate", "Candidate Leak"}:
        return 1 if raw_value.upper() == "YES" else 0
    if label == "Final Result":
        return 1 if raw_value.upper() == "LEAK" else 0
    if label == "LED":
        return 1 if raw_value.upper() == "ON" else 0
    if label == "Leak Count":
        return int(raw_value)
    return float(raw_value.split()[0])


def main():
    args = parse_args()

    try:
      ser = serial.Serial(args.port, args.baud, timeout=0.05)
    except serial.SerialException as exc:
      print(f"Could not open serial port {args.port}: {exc}", file=sys.stderr)
      sys.exit(1)

    time.sleep(2)
    print(f"Connected to {args.port} at {args.baud} baud")

    history = args.history
    x_values = deque(maxlen=history)
    sample_idx = 0

    data = {
        "sound_level": deque(maxlen=history),
        "peak_frequency": deque(maxlen=history),
        "spectral_center": deque(maxlen=history),
        "zero_cross_rate": deque(maxlen=history),
        "leak_probability": deque(maxlen=history),
        "audio_gate": deque(maxlen=history),
        "model_gate": deque(maxlen=history),
        "vibration_gate": deque(maxlen=history),
        "candidate_leak": deque(maxlen=history),
        "leak_count": deque(maxlen=history),
        "final_result": deque(maxlen=history),
        "led": deque(maxlen=history),
    }

    current_block = {}
    pattern = re.compile(r"^\s*([^:]+?)\s*:\s*(.+?)\s*$")

    fig, axes = plt.subplots(4, 1, figsize=(11, 9), sharex=True)
    fig.suptitle("ESP32 Leak Detection Live Plot")

    sound_line, = axes[0].plot([], [], label="Sound Level")
    peak_line, = axes[0].plot([], [], label="Peak Frequency")
    zcr_line, = axes[1].plot([], [], label="Zero Cross Rate")
    prob_line, = axes[1].plot([], [], label="Leak Probability")
    leak_count_line, = axes[2].plot([], [], label="Leak Count")
    candidate_line, = axes[3].plot([], [], label="Candidate Leak")
    result_line, = axes[3].plot([], [], label="Final Result")
    led_line, = axes[3].plot([], [], label="LED")

    axes[0].set_ylabel("Audio")
    axes[1].set_ylabel("Prob / ZCR")
    axes[2].set_ylabel("Count")
    axes[3].set_ylabel("State")
    axes[3].set_xlabel("Sample")

    axes[0].legend(loc="upper left")
    axes[1].legend(loc="upper left")
    axes[2].legend(loc="upper left")
    axes[3].legend(loc="upper left")
    axes[3].set_ylim(-0.1, 1.1)

    def append_block(block):
        nonlocal sample_idx
        required = {"sound_level", "peak_frequency", "zero_cross_rate", "leak_probability", "leak_count", "candidate_leak", "final_result", "led"}
        if not required.issubset(block.keys()):
            return

        x_values.append(sample_idx)
        sample_idx += 1

        for key in data:
            value = block.get(key)
            if value is None:
                if len(data[key]) > 0:
                    value = data[key][-1]
                else:
                    value = 0
            data[key].append(value)

    def poll_serial(_frame):
        while True:
            raw = ser.readline()
            if not raw:
                break

            try:
                line = raw.decode("utf-8", errors="ignore").strip()
            except Exception:
                continue

            if not line:
                if current_block:
                    append_block(current_block.copy())
                    current_block.clear()
                continue

            match = pattern.match(line)
            if not match:
                continue

            label, value = match.groups()
            if label in FIELDS:
                key = FIELDS[label]
                try:
                    current_block[key] = parse_value(label, value)
                except ValueError:
                    pass

        if not x_values:
            return (
                sound_line,
                peak_line,
                zcr_line,
                prob_line,
                leak_count_line,
                candidate_line,
                result_line,
                led_line,
            )

        xs = list(x_values)
        sound_line.set_data(xs, list(data["sound_level"]))
        peak_line.set_data(xs, list(data["peak_frequency"]))
        zcr_line.set_data(xs, list(data["zero_cross_rate"]))
        prob_line.set_data(xs, list(data["leak_probability"]))
        leak_count_line.set_data(xs, list(data["leak_count"]))
        candidate_line.set_data(xs, list(data["candidate_leak"]))
        result_line.set_data(xs, list(data["final_result"]))
        led_line.set_data(xs, list(data["led"]))

        for ax in axes:
            ax.relim()
            ax.autoscale_view()

        axes[3].set_ylim(-0.1, 1.1)

        return (
            sound_line,
            peak_line,
            zcr_line,
            prob_line,
            leak_count_line,
            candidate_line,
            result_line,
            led_line,
        )

    anim = FuncAnimation(fig, poll_serial, interval=100, blit=False, cache_frame_data=False)

    try:
        plt.tight_layout()
        plt.show()
    finally:
        ser.close()


if __name__ == "__main__":
    main()
