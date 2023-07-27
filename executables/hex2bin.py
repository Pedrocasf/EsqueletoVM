h = open("zephyr.hex", "r")
b = open("zephyr.bin", "wb")
for line in h:
    print(line)
    b.write(bytes.fromhex(line)[::-1])