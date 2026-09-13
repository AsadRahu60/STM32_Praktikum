import serial

PORT = '/dev/tty.usbmodem2111303'
BAUD = 115200
TOTAL = 500

def prbs7_next(state):
    bit = ((state >> 6) ^ (state >> 5)) & 1
    return ((state << 1) | bit) & 0x7F

ser = serial.Serial(PORT, BAUD, timeout=2)

# Collect raw values first
print("Collecting...")
values = []
while len(values) < TOTAL:
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    if line.startswith('PRBS:'):
        values.append(int(line.split(':')[1]))
ser.close()

# Lock onto sequence at values[0] — no sync needed
state    = values[0]   # wherever we joined, that's our starting point
bit_errors = 0
total_bits = 0

print(f"{'PKT':>6} {'RX':>6} {'EXP':>6} {'XOR':>8} {'BER':>12}")
print('-' * 45)

for i in range(1, len(values)):
    expected  = prbs7_next(state)
    received  = values[i]
    xor       = received ^ expected
    errs      = bin(xor).count('1')
    bit_errors += errs
    total_bits += 7
    state = expected   # advance expected regardless of errors

    if i % 50 == 0 or errs > 0:
        ber = bit_errors / total_bits
        print(f"{i:>6} {received:>6} {expected:>6} {xor:>08b} {ber:>12.2e}")

print(f"\n=== RESULT ===")
print(f"Total bits:  {total_bits}")
print(f"Bit errors:  {bit_errors}")
print(f"BER:         {bit_errors/total_bits:.4e}")