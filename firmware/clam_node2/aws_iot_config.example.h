#ifndef AWS_IOT_CONFIG_H
#define AWS_IOT_CONFIG_H

// Copy this file to aws_iot_config.h for local/private use only.
// Do not commit real credentials or certificates.

#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASS "your-wifi-password"
#define AWS_END "your-iot-endpoint.amazonaws.com"
#define DEV_ID "node02"

static const char ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
YOUR_ROOT_CA_HERE
-----END CERTIFICATE-----
)EOF";

static const char DEVICE_CERT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
YOUR_DEVICE_CERT_HERE
-----END CERTIFICATE-----
)KEY";

static const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
YOUR_PRIVATE_KEY_HERE
-----END RSA PRIVATE KEY-----
)KEY";

#endif
