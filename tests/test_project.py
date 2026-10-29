from pathlib import Path
import unittest,subprocess,tempfile,json,sys,re,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
class ProjectTests(unittest.TestCase):
 def compile(self,test,sources):
  with tempfile.TemporaryDirectory() as d:
   binary=str(Path(d)/'test')
   subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'main'),str(ROOT/'tests'/test)]+[str(ROOT/'main'/s) for s in sources]+['-o',binary],check=True)
   return subprocess.run([binary],capture_output=True,text=True,check=True).stdout
 def test_cover(self):self.compile('test_qimen_cover.c',['qimen_cover.c'])
 def test_navigation(self):self.compile('test_qimen_navigation.c',['qimen_navigation.c'])
 def test_protocol(self):self.compile('test_qimen_protocol.c',['reader_local.c','reader_radio.c','qimen_cover.c'])
 def test_status_articles(self):self.compile('test_reader_qimen_status.c',['reader_qimen.c','reader_qimen_status.c','reader_qimen_text.c','reader_qimen_notes.c'])
 def test_clock(self):self.compile('test_reader_clock.c',['reader_clock.c'])
 def test_pairing(self):self.compile('test_reader_pairing.c',['reader_pairing.c'])
 def test_history(self):
  cases=json.loads((ROOT/'tests/qimen_history_fixtures.json').read_text())
  with tempfile.TemporaryDirectory() as d:
   binary=str(Path(d)/'history');subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'main'),str(ROOT/'tests/test_history.c'),str(ROOT/'main/reader_qimen.c'),'-o',binary],check=True)
   rows=[json.loads(line) for line in subprocess.check_output([binary]+[str(c['epoch']) for c in cases],text=True).splitlines()];self.assertEqual(len(rows),len(cases))
   for case,got in zip(cases,rows):
    for key in ('civil','siZhu','term'):self.assertEqual(got[key],case[key],(case['epoch'],key))
 def test_calendar(self):
  with tempfile.TemporaryDirectory() as d:
   binary=str(Path(d)/'test');subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I'+str(ROOT/'main'),str(ROOT/'tests/test_reader_qimen.c'),str(ROOT/'main/reader_qimen.c'),'-o',binary],check=True)
   website=json.loads((ROOT/'tests/qimen_openfate_fixtures.json').read_text());calendar=json.loads((ROOT/'tests/qimen_calendar_fixtures.json').read_text())
   epochs=[str(int(datetime.datetime.fromisoformat(c['date']+'T'+c['time']+'+08:00').timestamp())) for c in website]+[str(c['epoch']) for c in calendar]
   rows=[json.loads(line) for line in subprocess.check_output([binary]+epochs,text=True).splitlines()];self.assertEqual(len(rows),len(epochs))
   for case,got in zip(website,rows):
    for key,value in got.items():
     expected=case['expected'][key]
     if key=='juShu':expected={k:expected[k] for k in value}
     self.assertEqual(value,expected)
   for case,got in zip(calendar,rows[len(website):]):self.assertEqual(got['siZhu'],case['siZhu']);self.assertEqual(got['juShu']['jieQiName'],case['term'])
 def test_radio_lifecycle(self):
  headers=['esp_partition.h','freertos/semphr.h','nvs.h','esp_err.h','esp_event.h','esp_http_server.h','esp_netif.h','esp_random.h','esp_wifi.h','esp_wifi_default.h','freertos/FreeRTOS.h','freertos/queue.h','freertos/task.h','esp_mac.h','esp_heap_caps.h','esp_log.h','esp_timer.h','host/ble_hs.h','host/ble_gap.h','host/util/util.h','nimble/nimble_port.h','services/gap/ble_svc_gap.h','services/gatt/ble_svc_gatt.h','os/os_mbuf.h']
  with tempfile.TemporaryDirectory() as directory:
   base=Path(directory)
   for name in headers:
    p=base/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('#include "'+str(ROOT/'tests/reader_stubs/platform.h')+'"\n')
   for name,sources in [('cover_store',['qimen_cover.c']),('storage',['reader_storage.c','reader_radio.c']),('network_runtime',['reader_radio.c']),('ble_runtime',['reader_pairing.c','reader_local.c','reader_radio.c','qimen_cover.c'])]:
    binary=str(base/name);subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',('-Wl,-dead_strip' if sys.platform=='darwin' else '-Wl,--gc-sections'),'-I'+str(base),'-I'+str(ROOT/'main'),str(ROOT/('tests/test_reader_'+name+'.c'))]+[str(ROOT/'main'/x) for x in sources]+['-o',binary],check=True);subprocess.run([binary],check=True)
 def test_font_inventory(self):
  text=''.join(p.read_text() for p in (ROOT/'main').glob('*.[ch]'));required={ord(c) for c in text if '\u3400'<=c<='\u9fff'}
  for font in ('reader_font','reader_chart_font'):
   f=(ROOT/f'assets/fonts/{font}.c').read_text();start=int(re.search(r'\.range_start=(\d+)',f)[1]);supported={start+int(n) for n in re.search(r'unicode_list\[\] = \{\s*([^}]+)',f)[1].split(',') if n.strip()};self.assertFalse(required-supported,font)
if __name__=='__main__':unittest.main()
