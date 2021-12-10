var temperature
var humidity
var time

window.onload = init

function init() {
	update_interval = window.setInterval(update, 1000)
	timer_interval = window.setInterval(timer, 1000)
	temperature = document.getElementById('temp')
	humidity = document.getElementById('humid')
	time = document.getElementById('time')
}

function update() {
	fetch('data.json', { method: 'get' })
		.then((res) => {
			return res.json()
		}).then((out) => {
			temperature.innerHTML = out.temperature
			humidity.innerHTML = out.humidity
			time.innerHTML = time.dateTime = out.time
		}).catch((err) => {
			throw err
		})
}

function timer() {
	var date = new Date()
	var split = time.innerHTML.split('.')[0].split(':')
	date.setHours(split[0], split[1], split[2], time.innerHTML.split('.')[1])
	date.setSeconds(date.getSeconds() + 1)

	var hours = date.getHours() % 24
	var minutes = date.getMinutes() % 60
	var seconds = date.getSeconds() % 60
	var milliseconds = date.getMilliseconds() % 1000
	var timeStr = ""
	if (hours < 10) {
		timeStr += '0'
	}
	timeStr += hours
	timeStr += ':'
	if (minutes < 10) {
		timeStr += '0'
	}
	timeStr += minutes
	timeStr += ':'
	if (seconds < 10) {
		timeStr += '0'
	}
	timeStr += seconds
	timeStr += '.'
	if (milliseconds < 100) {
		timeStr += '0'
		if (milliseconds < 10) {
			timeStr += '0'
		}
	}
	timeStr += milliseconds
	time.innerHTML = time.dateTime = timeStr
}
