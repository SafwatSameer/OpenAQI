// functions/mqtt-subscribe-handler/index.js
const AWS = require('aws-sdk');
const dynamoDB = new AWS.DynamoDB.DocumentClient();

exports.handler = async (event) => {
    console.log('Received event:', JSON.stringify(event, null, 2));
    
    try {
        // Get the table name from environment variables
        const tableName = process.env.ThingDataTable || 'ThingDataTable';
        
        // Create timestamp if not present
        const timestamp = new Date().toISOString();
        
        // Prepare the DynamoDB item
        const item = {
            clientId: event.clientId,
            createdAt: timestamp,
            humidity: event.humidity,
            temperature: event.temperature,
            pm1: event.pm1,
            pm25: event.pm25,
            pm10: event.pm10
        };
        
        // Add date and time if they exist in the event
        if (event.date) {
            item.date = event.date;
        }
        
        if (event.time) {
            item.time = event.time;
        }
        
        // Create combined datetime field for better sorting/filtering
        if (event.date && event.time) {
            item.datetime = `${event.date}T${event.time}`;
        }
        
        console.log('Saving item to DynamoDB:', JSON.stringify(item, null, 2));
        
        // Save to DynamoDB
        await dynamoDB.put({
            TableName: tableName,
            Item: item
        }).promise();
        
        console.log('Successfully saved item to DynamoDB');
        return { statusCode: 200, body: 'Success' };
    } catch (error) {
        console.error('Error saving to DynamoDB:', error);
        return { statusCode: 500, body: JSON.stringify(error) };
    }
};