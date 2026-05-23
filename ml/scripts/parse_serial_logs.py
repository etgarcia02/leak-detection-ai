from __future__ import annotations

import argparse
import csv
import re
from pathlib import Path
from statistics import mean


PROJECT_ROOT = Path(__file__).resolve().parents[2]

PAIR_RE = re.compile(r"([A-Za-z]+):([^\t\r\n ]+)")

SERIAL_FIELDS = [
    "SoundLevel",
    "PeakFreq",
    "SpectralCenter",
    "ZeroCross",
    "LeakProb",
    "AudioGate",
    "ModelGate",
    "VibrationGate",
    "CandidateLeak",
    "LeakCount",
    "FinalLeak",
    "LED",
]


def infer_label(stem: str) -> str:
    name = stem.lower()
    for label in ("no_leak", "small_leak", "medium_leak", "large_leak"):
        if label in name:
            return label
    if "leak" in name:
        return "leak"
    return "unknown"


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def to_float(value: str) -> float | None:
    try:
        return float(value)
    except ValueError:
        return None


def to_int(value: float) -> int:
    return int(round(value))


def parse_serial_file(path: Path) -> tuple[list[dict], int]:
    rows: list[dict] = []
    skipped_lines = 0

    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8", errors="ignore").splitlines(), start=1):
        pairs = dict(PAIR_RE.findall(raw_line))
        if "SoundLevel" not in pairs:
            skipped_lines += 1
            continue

        parsed: dict[str, float] = {}
        missing = False
        for field in SERIAL_FIELDS:
            value = pairs.get(field)
            if value is None:
                missing = True
                break
            numeric_value = to_float(value)
            if numeric_value is None:
                missing = True
                break
            parsed[field] = numeric_value

        if missing:
            skipped_lines += 1
            continue

        parsed["line_number"] = float(line_number)
        rows.append(parsed)

    return rows, skipped_lines


def validate_row(
    row: dict,
    *,
    saturation_threshold: float,
    low_signal_threshold: float,
    low_signal_prob_threshold: float,
) -> tuple[bool, str]:
    reasons: list[str] = []

    sound_level = float(row["SoundLevel"])
    peak_freq = float(row["PeakFreq"])
    spectral_center = float(row["SpectralCenter"])
    leak_prob = float(row["LeakProb"])

    if sound_level <= 0:
        reasons.append("sound_level_zero")
    if peak_freq == 0 and spectral_center == 0:
        reasons.append("empty_spectrum")
    if sound_level >= saturation_threshold:
        reasons.append("saturated")
    if sound_level < low_signal_threshold and leak_prob >= low_signal_prob_threshold:
        reasons.append("low_signal_high_prob")

    return (len(reasons) == 0, ";".join(reasons))


def build_output_row(source_file: str, session_id: str, raw_label: str, row_index: int, row: dict) -> dict:
    return {
        "source_file": source_file,
        "session_id": session_id,
        "raw_label": raw_label,
        "row_index": row_index,
        "line_number": int(row["line_number"]),
        "sound_level": float(row["SoundLevel"]),
        "peak_freq_hz": float(row["PeakFreq"]),
        "spectral_center_hz": float(row["SpectralCenter"]),
        "zero_crossing_rate": float(row["ZeroCross"]),
        "leak_prob": float(row["LeakProb"]),
        "audio_gate": to_int(float(row["AudioGate"])),
        "model_gate": to_int(float(row["ModelGate"])),
        "vibration_gate": to_int(float(row["VibrationGate"])),
        "candidate_leak": to_int(float(row["CandidateLeak"])),
        "leak_count": to_int(float(row["LeakCount"])),
        "final_leak": to_int(float(row["FinalLeak"])),
        "led": to_int(float(row["LED"])),
    }


def write_csv(path: Path, rows: list[dict], fieldnames: list[str]) -> None:
    ensure_parent(path)
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def safe_mean(rows: list[dict], key: str) -> float:
    if not rows:
        return 0.0
    return mean(float(row[key]) for row in rows)


def safe_max(rows: list[dict], key: str) -> float:
    if not rows:
        return 0.0
    return max(float(row[key]) for row in rows)


def summarize_session(
    source_file: str,
    session_id: str,
    raw_label: str,
    raw_rows: list[dict],
    clean_rows: list[dict],
    skipped_lines: int,
) -> dict:
    invalid_rows = len(raw_rows) - len(clean_rows)
    quality_flag = "clean" if invalid_rows == 0 and skipped_lines == 0 else "usable_with_cleaning"

    return {
        "source_file": source_file,
        "session_id": session_id,
        "raw_label": raw_label,
        "quality_flag": quality_flag,
        "parsed_rows": len(raw_rows),
        "valid_rows": len(clean_rows),
        "invalid_rows": invalid_rows,
        "skipped_non_data_lines": skipped_lines,
        "mean_sound_level": round(safe_mean(clean_rows, "sound_level"), 6),
        "mean_peak_freq_hz": round(safe_mean(clean_rows, "peak_freq_hz"), 6),
        "mean_spectral_center_hz": round(safe_mean(clean_rows, "spectral_center_hz"), 6),
        "mean_zero_crossing_rate": round(safe_mean(clean_rows, "zero_crossing_rate"), 6),
        "mean_leak_prob": round(safe_mean(clean_rows, "leak_prob"), 6),
        "max_leak_prob": round(safe_max(clean_rows, "leak_prob"), 6),
        "model_gate_count": sum(int(row["model_gate"]) for row in clean_rows),
        "candidate_leak_count": sum(int(row["candidate_leak"]) for row in clean_rows),
        "max_leak_count": int(safe_max(clean_rows, "leak_count")),
        "max_final_leak": int(safe_max(clean_rows, "final_leak")),
        "high_prob_rows_ge_0_90": sum(1 for row in clean_rows if float(row["leak_prob"]) >= 0.90),
    }


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Parse ESP32 serial leak logs into raw rows, cleaned rows, and a summary CSV."
    )
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=PROJECT_ROOT / "docs" / "raw_serial_logs",
        help="Directory containing raw serial logs as .txt or .log files.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=PROJECT_ROOT / "docs" / "parsed_serial_logs",
        help="Directory to store parsed CSV outputs.",
    )
    parser.add_argument(
        "--saturation-threshold",
        type=float,
        default=32000.0,
        help="Rows at or above this sound level are treated as saturated and invalid.",
    )
    parser.add_argument(
        "--low-signal-threshold",
        type=float,
        default=100.0,
        help="Rows below this sound level can be flagged as invalid when leak probability is pinned high.",
    )
    parser.add_argument(
        "--low-signal-prob-threshold",
        type=float,
        default=0.99,
        help="Rows below the low-signal threshold and at or above this probability are treated as invalid.",
    )
    args = parser.parse_args()

    input_dir = args.input_dir.resolve()
    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    input_paths = sorted(
        [path for path in input_dir.iterdir() if path.is_file() and path.suffix.lower() in {".txt", ".log"}]
    )
    if not input_paths:
        raise SystemExit(f"No .txt or .log files found in {input_dir}")

    row_fieldnames = [
        "source_file",
        "session_id",
        "raw_label",
        "row_index",
        "line_number",
        "sound_level",
        "peak_freq_hz",
        "spectral_center_hz",
        "zero_crossing_rate",
        "leak_prob",
        "audio_gate",
        "model_gate",
        "vibration_gate",
        "candidate_leak",
        "leak_count",
        "final_leak",
        "led",
        "is_valid",
        "invalid_reason",
    ]

    summary_rows: list[dict] = []
    combined_raw_rows: list[dict] = []
    combined_clean_rows: list[dict] = []

    for input_path in input_paths:
        session_id = input_path.stem
        raw_label = infer_label(session_id)
        parsed_rows, skipped_lines = parse_serial_file(input_path)

        output_rows: list[dict] = []
        clean_rows: list[dict] = []
        for row_index, parsed_row in enumerate(parsed_rows, start=1):
            output_row = build_output_row(input_path.name, session_id, raw_label, row_index, parsed_row)
            is_valid, invalid_reason = validate_row(
                parsed_row,
                saturation_threshold=args.saturation_threshold,
                low_signal_threshold=args.low_signal_threshold,
                low_signal_prob_threshold=args.low_signal_prob_threshold,
            )
            output_row["is_valid"] = int(is_valid)
            output_row["invalid_reason"] = invalid_reason
            output_rows.append(output_row)
            if is_valid:
                clean_rows.append(output_row)

        raw_output_path = output_dir / f"{session_id}_rows.csv"
        clean_output_path = output_dir / f"{session_id}_clean.csv"
        write_csv(raw_output_path, output_rows, row_fieldnames)
        write_csv(clean_output_path, clean_rows, row_fieldnames)

        combined_raw_rows.extend(output_rows)
        combined_clean_rows.extend(clean_rows)
        summary_rows.append(
            summarize_session(
                source_file=input_path.name,
                session_id=session_id,
                raw_label=raw_label,
                raw_rows=output_rows,
                clean_rows=clean_rows,
                skipped_lines=skipped_lines,
            )
        )

    summary_fieldnames = [
        "source_file",
        "session_id",
        "raw_label",
        "quality_flag",
        "parsed_rows",
        "valid_rows",
        "invalid_rows",
        "skipped_non_data_lines",
        "mean_sound_level",
        "mean_peak_freq_hz",
        "mean_spectral_center_hz",
        "mean_zero_crossing_rate",
        "mean_leak_prob",
        "max_leak_prob",
        "model_gate_count",
        "candidate_leak_count",
        "max_leak_count",
        "max_final_leak",
        "high_prob_rows_ge_0_90",
    ]

    write_csv(output_dir / "serial_log_summary.csv", summary_rows, summary_fieldnames)
    write_csv(output_dir / "serial_log_rows_combined.csv", combined_raw_rows, row_fieldnames)
    write_csv(output_dir / "serial_log_clean_combined.csv", combined_clean_rows, row_fieldnames)

    print(f"Parsed {len(input_paths)} log file(s) from {input_dir}")
    print(f"Wrote outputs to {output_dir}")


if __name__ == "__main__":
    main()
