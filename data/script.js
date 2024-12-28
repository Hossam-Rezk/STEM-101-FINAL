// script.js

// Replace with your ESP32's IP address (where it serves /data)
const ESP32_URL = "http://<ESP32_IP>/data"; 
// e.g. "http://192.168.1.50/data"

const tempCtx = document.getElementById('temperatureChart').getContext('2d');
const tempChart = new Chart(tempCtx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Temperature (°C)',
            data: [],
            borderColor: 'rgba(255, 99, 132, 1)',
            backgroundColor: 'rgba(255, 99, 132, 0.2)',
            borderWidth: 2,
            fill: true,
            tension: 0.4
        }]
    },
    options: {
        responsive: true,
        scales: {
            x: { title: { display: true, text: 'Time' } },
            y: {
                title: { display: true, text: 'Temperature (°C)' },
                ticks: { stepSize: 1 }
            }
        }
    }
});

const humCtx = document.getElementById('humidityChart').getContext('2d');
const humChart = new Chart(humCtx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Humidity (%)',
            data: [],
            borderColor: 'rgba(54, 162, 235, 1)',
            backgroundColor: 'rgba(54, 162, 235, 0.2)',
            borderWidth: 2,
            fill: true,
            tension: 0.4
        }]
    },
    options: {
        responsive: true,
        scales: {
            x: { title: { display: true, text: 'Time' } },
            y: { title: { display: true, text: 'Humidity (%)' } }
        }
    }
});

const mqCtx = document.getElementById('mq135Chart').getContext('2d');
const mqChart = new Chart(mqCtx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Air Quality (MQ135)',
            data: [],
            borderColor: 'rgba(75, 192, 192, 1)',
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderWidth: 2,
            fill: true,
            tension: 0.4
        }]
    },
    options: {
        responsive: true,
        scales: {
            x: { title: { display: true, text: 'Time' } },
            y: { title: { display: true, text: 'Sensor Value' } }
        }
    }
});

const dustCtx = document.getElementById('dustSensorChart').getContext('2d');
const dustChart = new Chart(dustCtx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Dust Concentration (μg/m³)',
            data: [],
            borderColor: 'rgba(153, 102, 255, 1)',
            backgroundColor: 'rgba(153, 102, 255, 0.2)',
            borderWidth: 2,
            fill: true,
            tension: 0.4
        }]
    },
    options: {
        responsive: true,
        scales: {
            x: { title: { display: true, text: 'Time' } },
            y: { title: { display: true, text: 'Dust (μg/m³)' } }
        }
    }
});

// Function to fetch sensor data from the ESP32 and update charts
function updateCharts() {
    fetch(ESP32_URL)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            const now = new Date().toLocaleTimeString();

            // Update Temperature Chart
            tempChart.data.labels.push(now);
            tempChart.data.datasets[0].data.push(data.temperature);
            if (tempChart.data.labels.length > 20) {
                tempChart.data.labels.shift();
                tempChart.data.datasets[0].data.shift();
            }
            tempChart.update();

            // Update Humidity Chart
            humChart.data.labels.push(now);
            humChart.data.datasets[0].data.push(data.humidity);
            if (humChart.data.labels.length > 20) {
                humChart.data.labels.shift();
                humChart.data.datasets[0].data.shift();
            }
            humChart.update();

            // Update Air Quality (MQ135) Chart
            mqChart.data.labels.push(now);
            mqChart.data.datasets[0].data.push(data.mq135);
            if (mqChart.data.labels.length > 20) {
                mqChart.data.labels.shift();
                mqChart.data.datasets[0].data.shift();
            }
            mqChart.update();

            // Update Dust Sensor Chart
            dustChart.data.labels.push(now);
            dustChart.data.datasets[0].data.push(data.dust);
            if (dustChart.data.labels.length > 20) {
                dustChart.data.labels.shift();
                dustChart.data.datasets[0].data.shift();
            }
            dustChart.update();

            // Update Stats
            document.getElementById('avg-temp').textContent =
              data.temperature != null ? `${data.temperature.toFixed(1)} °C` : "N/A";
            document.getElementById('avg-hum').textContent =
              data.humidity != null ? `${data.humidity.toFixed(1)} %` : "N/A";
            document.getElementById('avg-air').textContent =
              data.mq135 != null ? data.mq135 : "N/A";
            document.getElementById('avg-dust').textContent =
              data.dust != null ? `${data.dust.toFixed(1)} μg/m³` : "N/A";

            // Display warning if thresholds are exceeded (example)
            const warningElement = document.getElementById('warning');
            if (data.temperature > 30 || data.humidity > 70 || data.mq135 > 300 || data.dust > 150) {
                warningElement.style.display = 'block';
                warningElement.textContent = 'Warning: Threshold exceeded!';
            } else {
                warningElement.style.display = 'none';
            }
        })
        .catch(error => console.error('Error fetching data:', error));
}

// Update charts every 2 seconds
setInterval(updateCharts, 2000);
