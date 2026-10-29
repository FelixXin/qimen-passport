/* Build-time generator only. Calendar source: lunar-javascript 1.7.7 (MIT). */
'use strict';
const fs=require('node:fs'),crypto=require('node:crypto'),path=require('node:path');
const source=path.resolve(process.argv[2]||'');
const expected='9750324bfe1aa63c146f8c72b1143df924466c11c8a5277d7d9225c541a18aaa';
if(crypto.createHash('sha256').update(fs.readFileSync(source)).digest('hex')!==expected)throw Error('Use the documented pinned lunar.js source');
const {Solar}=require(source);
const names=['小寒','大寒','立春','雨水','惊蛰','春分','清明','谷雨','立夏','小满','芒种','夏至','小暑','大暑','立秋','处暑','白露','秋分','寒露','霜降','立冬','小雪','大雪','DONG_ZHI'];
let rows=[];
for(let year=1899;year<=2100;year++){
 const table=Solar.fromYmdHms(year,6,1,12,0,0).getLunar().getJieQiTable();
 const epochs=names.map(name=>{const s=table[name];if(s.getYear()!==year)throw Error('Unexpected year');return Date.UTC(s.getYear(),s.getMonth()-1,s.getDay(),s.getHour(),s.getMinute(),s.getSecond())/1000-28800;});
 if(epochs.some((n,i)=>i&&n<=epochs[i-1]))throw Error('Unordered terms');
 rows.push('    {'+epochs.map(n=>'INT64_C('+n+')').join(',')+'}, /* '+year+' */');
}
const output=process.argv[3];
fs.writeFileSync(output,'/* Generated UTC seconds, 1899–2100; lunar-javascript 1.7.7 (MIT).\n * Source SHA256: '+expected+'\n * Regenerate with tools/generate_qimen_terms.js; see assets/calendar/README.md. */\nstatic const int64_t solar_terms[202][24] = {\n'+rows.join('\n')+'\n};\n');
console.log('Solar terms: 202 years × 24, UTC seconds');
