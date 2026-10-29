'use strict';
const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(require('node:path').join(__dirname,'../assets/web/device.html'),'utf8');const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
const els=new Map(),calls=[];let failure=false,stale=false;const dom=id=>{if(!els.has(id))els.set(id,{value:'',textContent:''});return els.get(id);};
const ctx=vm.createContext({document:{getElementById:dom,hidden:false},TextEncoder,Date,Error,Number,Array,AbortController,setTimeout:(fn,ms)=>{if(ms===1000){fn();return 0;}return setTimeout(fn,ms);},clearTimeout,setInterval:()=>0,
 fetch:async(url,args)=>{calls.push({url,args});if(failure)return {ok:false,text:async()=>'invalid key'};
  const command=args.body.split('\t')[0];let reply='QIMEN\tOK\t'+command;
  if(command==='HELLO')reply+='\t0.1.0';if(command==='GET')reply+='\t'+Math.floor(Date.now()/1000);
  if(command==='NET')reply+='\t1\t1\t0\t1\t'+(stale?99:7)+'\tCONNECTED\t192.168.1.2';
  if(command==='STATS')reply+='\t80000\t60000\t70000\t5000';if(command==='WIFI')reply+='\t7';
  return {ok:true,text:async()=>reply};}});
vm.runInContext(script,ctx);
(async()=>{
 dom('key').value='bad';await dom('connect').onclick();assert.match(dom('status').textContent,/八位/);assert.equal(calls.length,0);
 dom('key').value='1234ABCD';await dom('connect').onclick();assert.match(dom('clock').textContent,/北京时间/);assert.match(dom('status').textContent,/80000/);
 assert(calls.every(c=>c.url==='/api/command'&&c.args.headers['X-Qimen-Key']==='1234ABCD'&&!c.args.body.includes('\n')));
 dom('ssid').value='测试网络';dom('password').value='password123';await dom('wifi').onclick();assert.equal(dom('password').value,'');assert.equal(dom('status').textContent,'连接并保存成功');assert(calls.some(c=>c.args.body.startsWith('WIFI\te6b58be8af95e7bd91e7bb9c\t')));
 stale=true;dom('password').value='password123';await dom('wifi').onclick();assert.match(dom('status').textContent,/请求已改变/);stale=false;
 failure=true;await dom('refresh').onclick();assert.equal(dom('status').textContent,'invalid key');
 assert(!/localStorage|api\/book|PROFILE|AVATAR/.test(html));
 console.log('Web: header-only key, no LF command, phone-time readback, UTF-8 Wi-Fi, ticket/save confirmation, stale request, errors and no personal/book routes PASS');
})().catch(e=>{console.error(e);process.exitCode=1;});
