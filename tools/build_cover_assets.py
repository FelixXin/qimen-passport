#!/usr/bin/env python3
"""Render the existing Taiji SVG geometry on a light paper background for LVGL."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
# Same viewBox/circles as assets/images/taiji-avatar.svg; only the outer paper changes.
scale=4;image=Image.new('RGB',(256*scale,256*scale),'#f6f4ed');draw=ImageDraw.Draw(image)
def circle(x,y,r,color):draw.ellipse(tuple(int(v*scale) for v in (x-r,y-r,x+r,y+r)),fill=color)
circle(128,128,112,'#10110e')
white=Image.new('RGB',image.size,'#10110e');d=ImageDraw.Draw(white);d.rectangle((0,0,128*scale,256*scale),fill='#f6f4ed');d.ellipse((72*scale,16*scale,184*scale,128*scale),fill='#f6f4ed');d.ellipse((72*scale,128*scale,184*scale,240*scale),fill='#10110e')
mask=Image.new('L',image.size,0);ImageDraw.Draw(mask).ellipse((16*scale,16*scale,240*scale,240*scale),fill=255);image.paste(white,(0,0),mask)
draw=ImageDraw.Draw(image);circle(128,72,14,'#10110e');circle(128,184,14,'#f6f4ed');draw.ellipse((16*scale,16*scale,240*scale,240*scale),outline='#b8b8a8',width=scale)
image=image.resize((64,64),Image.Resampling.LANCZOS);image.save(root/'assets/images/taiji-light.png')
values=[]
for red,green,blue in image.getdata():
 value=((red>>3)<<11)|((green>>2)<<5)|(blue>>3);values.extend((value&255,value>>8))
text='/* Taiji SVG geometry, RGB565 little-endian, paper variant. */\n#include "lvgl.h"\nstatic const uint8_t pixels[]={\n'
text+='\n'.join(','.join('0x%02x'%n for n in values[i:i+24])+',' for i in range(0,len(values),24))
text+='\n};\nconst lv_image_dsc_t taiji_light={.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,.w=64,.h=64,.stride=128},.data_size=sizeof(pixels),.data=pixels};\n'
(root/'assets/images/taiji_light.c').write_text(text)
