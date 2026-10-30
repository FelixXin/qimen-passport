'use strict';
const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(require('node:path').join(__dirname,'../assets/web/device.html'),'utf8');const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
const els=new Map(),calls=[];let failure=false,stale=false;const intervals=[];let netState=4,netReason=0,netErrorCode=0;const dom=id=>{if(!els.has(id))els.set(id,{value:'',textContent:''});return els.get(id);};
const ctx=vm.createContext({document:{getElementById:dom,hidden:false},TextEncoder,TextDecoder,Uint8Array,Date,Error,Number,Array,AbortController,setTimeout:(fn,ms)=>{if(ms===1000){fn();return 0;}return setTimeout(fn,ms);},clearTimeout,setInterval:(fn,ms)=>{intervals.push({fn,ms});return 0;},
 fetch:async(url,args)=>{calls.push({url,args});if(failure)return {ok:false,text:async()=>'invalid key'};
  const command=args.body.split('\t')[0];let reply='QIMEN\tOK\t'+command;
  if(command==='HELLO')reply+='\t0.3.0';if(command==='COVERINFO')reply+='\t\t0\t1';if(command==='STYLEINFO')reply+='\t0\t0';if(command==='GET')reply+='\t'+Math.floor(Date.now()/1000);
  if(command==='NETINFO')reply+='\t1\t1\t2\t726f75746572\t726f75746572\t'+netState+'\t192.168.1.2\t-52\t'+netReason+'\t0\t51696d656e2d50617373706f7274\t0\t'+netErrorCode;
  if(command==='RECONNECT')reply+='\t7';
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
 failure=false;dom('name').value='学易';await dom('save-name').onclick();assert.equal(dom('status').textContent,'名称已保存到设备');assert(calls.some(c=>c.args.body==='NAME\te5ada6e69893'));
 failure=false;await dom('reset-cover').onclick();assert.match(dom('status').textContent,/太极/);
 dom('cast-time').value='1900-01-01T00:00';const before=calls.length;await dom('cast-at').onclick();assert.equal(calls[before].args.body,'CASTAT\t-2209017600');assert.equal(calls.length,before+1);
 dom('cast-time').value='1900-02-29T12:00';await dom('cast-at').onclick();assert.match(dom('status').textContent,/不存在/);
 dom('cast-time').value='2000-02-29T12:00';await dom('cast-at').onclick();assert.match(dom('status').textContent,/时钟保持不变/);
 for(const date of ['1899-12-31T23:59','2100-01-01T00:00','2024-13-01T00:00']){dom('cast-time').value=date;const n=calls.length;await dom('cast-at').onclick();assert.equal(calls.length,n);}
 dom('style').value='1';await dom('save-style').onclick();assert(calls.some(c=>c.args.body==='STYLE\t1'));
 const invalidAp=['1234567','a'.repeat(64),'中'.repeat(22)];for(const password of invalidAp){dom('ap-password').value=password;const before=calls.length;await dom('save-ap').onclick();assert.equal(calls.length,before);assert.match(dom('status').textContent,/8–63/);}
 dom('ap-password').value='12345678';const apBefore=calls.length;await dom('save-ap').onclick();assert.equal(calls.length,apBefore+1);assert.equal(calls[apBefore].args.body,'AP\t51696d656e2d50617373706f7274\t3132333435363738');assert.equal(dom('ap-password').value,'');assert.match(dom('ap-status').textContent,/关闭再开启/);
 failure=true;dom('ap-password').value='new-pass123';await dom('save-ap').onclick();assert.equal(dom('ap-password').value,'new-pass123');assert.equal(dom('status').textContent,'invalid key');failure=false;
 dom('ap-name').value='new-hotspot';dom('ap-name').oninput();dom('ap-password').value='';await dom('save-ap').onclick();assert(calls.some(c=>c.args.body==='AP\t6e65772d686f7473706f74\t'));
 const poll=intervals.find(x=>x.ms===3000);assert(poll);dom('page-network').hidden=false;await poll.fn();assert.match(dom('network-status').textContent,/路由器已连接/);assert.match(dom('network-status').textContent,/2 台/);assert.match(dom('network-status').textContent,/router/);
 dom('ap-name').value='editing-name';dom('ap-name').oninput();await poll.fn();assert.equal(dom('ap-name').value,'editing-name');
 netState=3;netReason=202;netErrorCode=1;await poll.fn();assert.match(dom('network-status').textContent,/重连中/);assert.match(dom('network-status').textContent,/认证失败/);
 netState=5;await poll.fn();assert.match(dom('network-status').textContent,/连接失败/);
 const beforeHidden=calls.length;ctx.document.hidden=true;await poll.fn();assert.equal(calls.length,beforeHidden);ctx.document.hidden=false;
 netState=4;netReason=netErrorCode=0;await dom('reconnect').onclick();assert(calls.some(c=>c.args.body==='RECONNECT'));assert.equal(dom('status').textContent,'连接并保存成功');
 failure=true;await poll.fn();assert.match(dom('network-status').textContent,/状态暂未更新/);assert.equal(dom('ap-name').value,'editing-name');failure=false;
 const crc=vm.runInContext('crc32(new TextEncoder().encode("123456789"))',ctx);assert.equal(crc,0xcbf43926);
 assert(!/localStorage|api\/book|PROFILE|AVATAR/.test(html));
 console.log('Web: header-only key, no LF command, phone-time readback, UTF-8 Wi-Fi, ticket/save confirmation, stale request, errors and no personal/book routes PASS');
})().catch(e=>{console.error(e);process.exitCode=1;});
