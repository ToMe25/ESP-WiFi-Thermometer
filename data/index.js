var temp_element
var humidity_element
var time_element
var update_interval
var timer_interval
var json_time
var update_time

function init(){
update_interval=window.setInterval(update,1000)
timer_interval=window.setInterval(timer,1000)
temp_element=document.getElementById('temp')
humidity_element=document.getElementById('humid')
time_element=document.getElementById('time')
json_time=parseTimeString(time_element.innerText)
update_time=Date.now()
}

function update(){
var options={method:'GET'}
var timeout
if(typeof(AbortController)=='function'){
const abort=new AbortController();
options.signal=abort.signal
timeout=setTimeout(()=>abort.abort(),3000);
}

fetch('data.json',options)
.then((res)=>{
if(timeout!=undefined){
clearTimeout(timeout)
}
return res.json()
}).then((out)=>{
temp_element.innerText=out.temperature
humidity_element.innerText=out.humidity
time_element.innerText=time_element.dateTime=out.time
json_time=parseTimeString(out.time)
update_time=Date.now()
}).catch((err)=>{
if(timeout!=undefined){
clearTimeout(timeout)
}
console.error('Error: ',err)
})
}

function timer(){
if(time_element.innerText=="Unknown"){
return
}

var date=new Date(Date.now()-update_time+json_time)
const hours=date.getUTCHours()
const minutes=date.getUTCMinutes()
const seconds=date.getUTCSeconds()
const milliseconds=date.getUTCMilliseconds()
var timeStr=""
if(hours<10){
timeStr+='0'
}
timeStr+=hours
timeStr+=':'
if(minutes<10){
timeStr+='0'
}
timeStr+=minutes
timeStr+=':'
if(seconds<10){
timeStr+='0'
}
timeStr+=seconds
timeStr+='.'
if(milliseconds<100){
timeStr+='0'
if(milliseconds<10){
timeStr+='0'
}
}
timeStr+=milliseconds
time_element.innerText=time_element.dateTime=timeStr
}

function parseTimeString(time){
if(typeof(time)!="string"){
console.error("Can't parse time object of type %s.",typeof (time))
return null
}

if(time=="Unknown"){
return null
}

const split=time.split('.')[0].split(':')
if(split.length!=3){
console.error('Date string "%s" is invalid!',time)
return null
}

var date=new Date(0)
date.setUTCHours(split[0],split[1],split[2],time.split('.')[1])
return date.getTime()
}

document.addEventListener('DOMContentLoaded',init)
