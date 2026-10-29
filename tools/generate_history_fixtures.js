'use strict';
const fs=require('node:fs'),{Solar}=require('./vendor/lunar.js');
function record(epoch){const d=new Date((epoch+28800)*1000);const civil=[d.getUTCFullYear(),d.getUTCMonth()+1,d.getUTCDate(),d.getUTCHours(),d.getUTCMinutes(),d.getUTCSeconds()];const l=Solar.fromYmdHms(...civil).getLunar();return {epoch,civil,siZhu:{year:l.getYearInGanZhiByLiChun(),month:l.getMonthInGanZhiExact(),day:l.getDayInGanZhi(),time:l.getTimeInGanZhi()},term:l.getPrevJieQi(false).getName()};}
const original=JSON.parse(fs.readFileSync(process.argv[2]));for(const item of original){const r=record(item.epoch);if(JSON.stringify(r.siZhu)!==JSON.stringify(item.siZhu)||r.term!==item.term)throw Error('Previous fixture mismatch '+item.epoch+JSON.stringify(r));}
const epochs=new Set([-2209017600,-1,0,4102415999]);
for(let y=1900;y<=2099;y++)for(const [m,d,h] of [[1,1,0],[2,28,12],[3,1,0],[6,15,13],[12,31,23]])epochs.add(Date.UTC(y,m-1,d,h,30,0)/1000-28800);
const names=['小寒','大寒','立春','雨水','惊蛰','春分','清明','谷雨','立夏','小满','芒种','夏至','小暑','大暑','立秋','处暑','白露','秋分','寒露','霜降','立冬','小雪','大雪','DONG_ZHI'];
for(const y of [1900,1901,1912,1949,1969,1970,1999,2000,2024,2099]){const table=Solar.fromYmdHms(y,6,1,12,0,0).getLunar().getJieQiTable();for(const name of names){const s=table[name],t=Date.UTC(s.getYear(),s.getMonth()-1,s.getDay(),s.getHour(),s.getMinute(),s.getSecond())/1000-28800;for(const delta of [-1,0,1])epochs.add(t+delta);}}
const records=Array.from(epochs).sort((a,b)=>a-b).map(record);fs.writeFileSync(process.argv[3],JSON.stringify(records,null,2)+'\n');console.log('Previous 516 fixtures matched; history fixtures: '+records.length);
