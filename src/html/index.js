var temp_elemen
var humidity_element
var time_element
var update_interval
var timer_interval
var json_time
var update_time

window.onload = init

/**
 * The function used to initialize this script.
 * 
 * The function to be called when the page finished loading, to initialize this script.
 */
function init() {
	update_interval = window.setInterval(update, 1000)
	timer_interval = window.setInterval(timer, 1000)
	temp_elemen = document.getElementById('temp')
	humidity_element = document.getElementById('humid')
	time_element = document.getElementById('time')
	json_time = parseTimeString(time_element.innerText)
	update_time = Date.now()
}

/**
 * The update function downloading new data from the ESP.
 * 
 * This function fetches new measurements from the ESP and updates the page.
 */
function update() {
	var options = { method: 'GET' }
	var timeout
	if (typeof (AbortController) == 'function') {
		const abort = new AbortController();
		options.signal = abort.signal
		timeout = setTimeout(() => abort.abort(), 3000);
	}

	fetch('data.json', options)
		.then((res) => {
			if (timeout != undefined) {
				clearTimeout(timeout)
			}
			return res.json()
		}).then((out) => {
			temp_elemen.innerText = out.temperature
			humidity_element.innerText = out.humidity
			time_element.innerText = time_element.dateTime = out.time
			json_time = parseTimeString(out.time)
			update_time = Date.now()
		}).catch((err) => {
			if (timeout != undefined) {
				clearTimeout(timeout)
			}
			console.error('Error: ', err)
		})
}

/**
 * The timer function updating the time since measurement every second.
 * 
 * A function that will be called once every second, and updates the time since the last measurement.
 */
function timer() {
	if (time_element.innerText == "Unknown") {
		return
	}

	var date = new Date(Date.now() - update_time + json_time)
	const hours = date.getUTCHours()
	const minutes = date.getUTCMinutes()
	const seconds = date.getUTCSeconds()
	const milliseconds = date.getUTCMilliseconds()
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
	time_element.innerText = time_element.dateTime = timeStr
}

/**
 * Parses the given time string to the number of milliseconds it represents.
 * 
 * This function parses a string in the format "HH:MM:SS.mmm" to a number.
 * The result is the number of milliseconds the time represents.
 * 
 * Will return null if time is "Unknown" or can not be parsed.
 * 
 * @param {string} time The string to parse.
 * @return {number} The parsed Date object.
 */
function parseTimeString(time) {
	if (typeof (time) != "string") {
		console.error("Can't parse time object of type %s.", typeof (time))
		return null
	}

	if (time == "Unknown") {
		return null
	}

	const split = time.split('.')[0].split(':')
	if (split.length != 3) {
		console.error('Date string "%s" is invalid!', time)
		return null
	}

	var date = new Date(0)
	date.setUTCHours(split[0], split[1], split[2], time.split('.')[1])
	return date.getTime()
}
