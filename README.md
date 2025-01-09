# mov2svg
convert movie file or png file(s) to sequence of SVG files

## 1. Usage
- Drag and Drop movie file (mp4) or png file(s) or folder(s) contains png file(s).
- It converts files to svg files into "svgs" folder.

black in white checkbox: enable to detect black object in white background which is opposite to default setting.
min_radius: minimum radius of objects to detect.
max_radius: maximum radius of objects to detect.
threshold: threshold value (0-255) to detect objects.
scale: changes scale of output svg files. (using larger size of images helps higher accuracy of detection)
simplify: simplify vector path of objects. (the higher value, the more simplified path)
