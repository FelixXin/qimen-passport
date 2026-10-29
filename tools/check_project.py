from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
private=['bairong1029','4C:11:AE:2F:CF:60','柏荣','冯鑫','FENGXIN','Obsidian Vault','/Users/']
for folder in ['main','assets','tests','docs']:
 for p in (root/folder).rglob('*'):
  if not p.is_file() or p.suffix not in ('.c','.h','.html','.md','.json','.inc'):continue
  text=p.read_text();assert not any(value in text for value in private),str(p)
  assert all(line==line.rstrip() for line in text.splitlines()),str(p)
main=''.join(p.read_text() for p in (root/'main').glob('*.[ch]'))
for marker in ['reader_core','reader_upload','spiffs','LOCAL_PROFILE','LOCAL_AVATAR','wordbooks','developer_avatar']:
 assert marker not in main,marker
assert 'spiffs_create_partition_image' not in (root/'main/CMakeLists.txt').read_text()
table=(root/'partitions.csv').read_text();assert '0x10000,0x370000' in table and '0x380000,0x80000' in table
print('Public-source and removed-feature checks: PASS')
