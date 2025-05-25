const mqtt = require('mqtt');

// this code isn't tested yet

// Create an MQTT client
const client = mqtt.connect('mqtt://localhost'); // by default, it will set its port to 1883 (default)

console.log("MQTT Server started. Waiting for messages...");

client.on('connect', () => {
    console.log('Connected to MQTT broker');
    
    client.subscribe('sensor/ultrasonic', (err) => {
        if (!err) {
        console.log('Subscribed to topic: sensor/ultrasonic');
        }
    });
});

client.on('message', (topic, message) => {
  // message is Buffer
    const status = message.toString();
    const timestamp = new Date().toISOString();
  
    console.log(`[${timestamp}] Object detected: ${status}`);
});

client.on('error', (err) => {
    console.error('MQTT error:', err);
});