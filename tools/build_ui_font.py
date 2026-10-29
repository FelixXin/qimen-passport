#!/usr/bin/env python3
"""Rebuild the compact UI font from a licensed Noto Sans SC font (Pillow/fontTools)."""
import argparse
from pathlib import Path
from PIL import ImageFont
from fontTools.ttLib import TTFont
from compress_reader_fonts import compress


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--font',required=True,type=Path)
    parser.add_argument('--license',required=True,type=Path)
    parser.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1])
    args=parser.parse_args()
    text=''.join(p.read_text() for p in (args.root/'main').glob('*.[ch]'))
    inventory=set(range(32,0x300))|{ord(c) for c in text if ord(c)>=32}
    source=TTFont(args.font);supported=source.getBestCmap();source.close()
    inventory=sorted(inventory&supported.keys())
    font=ImageFont.truetype(str(args.font),12);ascent,descent=font.getmetrics()
    bitmap=bytearray();glyphs=[(0,0,0,0,0,0)]
    for code in inventory:
        mask,offset=font.getmask2(chr(code),mode='L',anchor='ls');pixels=list(mask);start=len(bitmap)
        for i in range(0,len(pixels),2):
            a=(pixels[i]*15+127)//255;b=(pixels[i+1]*15+127)//255 if i+1<len(pixels) else 0
            bitmap.append(a<<4|b)
        glyphs.append((start,round(font.getlength(chr(code))*16),mask.size[0],mask.size[1],offset[0],-offset[1]-mask.size[1]))
    lines=['/* Generated from Noto Sans SC, 12 px / 4 bpp. Source: https://github.com/notofonts/noto-cjk','\n'.join(line.rstrip() for line in args.license.read_text().replace('*/','* /').splitlines()),'*/','#include "lvgl.h"','static const uint8_t glyph_bitmap[] = {']
    lines += [','.join(f'0x{x:02x}' for x in bitmap[i:i+24])+',' for i in range(0,len(bitmap),24)]
    lines += ['};','static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {']
    lines += ['{.bitmap_index=%d,.adv_w=%d,.box_w=%d,.box_h=%d,.ofs_x=%d,.ofs_y=%d},'%g for g in glyphs]
    start=inventory[0]
    lines += ['};','static const uint16_t unicode_list[] = {',','.join(str(c-start) for c in inventory),'};','static const lv_font_fmt_txt_cmap_t cmaps[] = {{',f'.range_start={start},.range_length={inventory[-1]-start+1},.glyph_id_start=1,',f'.unicode_list=unicode_list,.glyph_id_ofs_list=NULL,.list_length={len(inventory)},.type=LV_FONT_FMT_TXT_CMAP_SPARSE_TINY','}};','static const lv_font_fmt_txt_dsc_t font_dsc = {','.glyph_bitmap=glyph_bitmap,.glyph_dsc=glyph_dsc,.cmaps=cmaps,.kern_dsc=NULL,','.kern_scale=0,.cmap_num=1,.bpp=4,.kern_classes=0,.bitmap_format=0};','const lv_font_t reader_chart_font = {','.get_glyph_dsc=lv_font_get_glyph_dsc_fmt_txt,.get_glyph_bitmap=lv_font_get_bitmap_fmt_txt,',f'.line_height={ascent+descent},.base_line={descent},.subpx=LV_FONT_SUBPX_NONE,','.underline_position=-2,.underline_thickness=1,.dsc=&font_dsc,.fallback=NULL};']
    target=args.root/'assets/fonts/reader_chart_font.c';target.write_text('\n'.join(lines)+'\n');old,new=compress(target)
    print(f'Compact UI font: {len(inventory)} glyphs, {old} -> {new} bytes, lossless PASS')


if __name__=='__main__':main()
