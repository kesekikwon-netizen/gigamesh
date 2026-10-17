# 기준 32 재판정용: 실제 글꼴 캡처 두 장의 화소 차이 띠를 찍는다.
# 사용: python -I rfdiff.py <JOB-006 shot_main_font.png> <JOB-007 shot_main_font.png>
import sys
from PIL import Image, ImageChops
a = Image.open(sys.argv[1]).convert('RGB'); b = Image.open(sys.argv[2]).convert('RGB')
print('size', a.size, b.size)
d = ImageChops.difference(a, b).convert('L').point(lambda v: 255 if v > 8 else 0)
W, H = d.size; px = d.load()
rows = [sum(1 for x in range(W) if px[x, y]) for y in range(H)]
start = None
for y in range(H + 1):
    c = rows[y] if y < H else 0
    if c and start is None: start = y
    if not c and start is not None:
        xs = [x for yy in range(start, y) for x in range(W) if px[x, yy]]
        print(f'band y {start}-{y-1} px {sum(rows[start:y])} x {min(xs)}-{max(xs)}'); start = None
def edges(img):
    p = img.load(); y = 400
    return [x for x in range(1, img.size[0]) if abs(sum(p[x, y]) - sum(p[x - 1, y])) > 60][:12]
print('edges006', edges(a)); print('edges007', edges(b))
