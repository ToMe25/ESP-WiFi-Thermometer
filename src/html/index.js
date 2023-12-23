var temperature
var humidity
var time
var update_interval
var timer_interval

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
			temperature.innerText = out.temperature
			humidity.innerText = out.humidity
			time.innerText = time.dateTime = out.time
		}).catch((err) => {
			console.error('Error: ', err)
		})
}

function timer() {
	if (time.innerText == "Unknown") {
		return
	}

	var date = new Date()
	const split = time.innerText.split('.')[0].split(':')
	if (split.length != 3) {
		console.error('Date string "%s" is invalid!', time.innerText)
		return
	}

	date.setHours(split[0], split[1], split[2], time.innerText.split('.')[1])
	date.setSeconds(date.getSeconds() + 1)

	const hours = date.getHours() % 24
	const minutes = date.getMinutes() % 60
	const seconds = date.getSeconds() % 60
	const milliseconds = date.getMilliseconds() % 1000
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
	time.innerText = time.dateTime = timeStr
}
