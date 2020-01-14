import time
import sys

running = True

while running:
    with open(".face.txt") as f:
        content = f.read()
    sys.stdout.write("\rFaces detected: " + content)
    sys.stdout.flush()
    time.sleep(0.1)
