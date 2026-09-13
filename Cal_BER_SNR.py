import serial
import time
import pandas as pd
import matplotlib.pyplot as plt

PORT = '/dev/tty.usbmodem2111303'   # adjust if needed
BAUD = 115200
SAMPLES = 50                         # collect this many packets then auto-stop

ser = serial.Serial(PORT, BAUD, timeout=2)
time.sleep(0.5)

# Storage lists — one entry per packet
counts  = []
errors  = []
bers    = []
adcs    = []
snrs    = []

print(f"Collecting {SAMPLES} samples... touch PA0 to GND to simulate errors")
print(f"{'COUNT':>8} {'ERRORS':>8} {'BER':>12} {'ADC':>8} {'SNR_dB':>8}")
print('-' * 52)

try:
    while len(counts) < SAMPLES:           # stop after SAMPLES packets
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        # Parse key:value pairs
        parts = {}
        for token in line.split():
            if ':' in token:
                k, v = token.split(':', 1)
                parts[k] = v

        try:
            count  = int(parts['COUNT'])
            error  = int(parts['BER_errors'])
            adc    = int(parts['ADC'])
        except (KeyError, ValueError):
            continue

        total_bits = count * 8
        ber    = error / total_bits if total_bits > 0 else 0.0
        snr_db = (adc / 4095.0) * 30.0

        # Store in lists
        counts.append(count)
        errors.append(error)
        bers.append(ber)
        adcs.append(adc)
        snrs.append(snr_db)

        print(f"{count:>8} {error:>8} {ber:>12.2e} {adc:>8} {snr_db:>8.2f}")

except KeyboardInterrupt:
    print("\nStopped early.")
finally:
    ser.close()

# ── Build DataFrame ──────────────────────────────
df = pd.DataFrame({
    'COUNT':  counts,
    'ERRORS': errors,
    'BER':    bers,
    'ADC':    adcs,
    'SNR_dB': snrs,
})

print(f"\nCollected {len(df)} rows")
print(df.describe())   # min, max, mean, std for every column

# ── Plot ─────────────────────────────────────────
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6))

# Top plot: BER over time
ax1.plot(df['COUNT'], df['BER'], color='red', label='BER')
ax1.set_xlabel('Packet count')
ax1.set_ylabel('BER')
ax1.set_title('Bit Error Rate over time')
ax1.legend()
ax1.grid(True)

# Bottom plot: SNR over time
ax2.plot(df['COUNT'], df['SNR_dB'], color='blue', label='SNR (dB)')
ax2.set_xlabel('Packet count')
ax2.set_ylabel('SNR (dB)')
ax2.set_title('SNR over time')
ax2.legend()
ax2.grid(True)

plt.tight_layout()
plt.savefig('ber_snr_plot.png')   # saves image to current folder
print("Plot saved: ber_snr_plot.png")
plt.show()