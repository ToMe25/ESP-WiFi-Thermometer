var temperature
var hunidity

window.onload = init

interval = window.setInterval(update, 1000)

function init() {
	temperature = document.getElementById('temp')
	humidity = document.getElementById('humid')
}

function update() {
	fetch('data.json', { method: 'get' })
		.then((res) => {
			return res.json()
		}).then((out) => {
			temperature.innerHTML = out.temperature
			humidity.innerHTML = out.humidity
		}).catch((err) => {
			throw err
		})
}
