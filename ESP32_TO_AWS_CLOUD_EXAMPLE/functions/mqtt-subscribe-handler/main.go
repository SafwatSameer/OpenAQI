package main

import (
  "context"
  "fmt"
  "time"
  "github.com/aws/aws-lambda-go/lambda"
  "github.com/aws/aws-sdk-go/aws"
  "github.com/aws/aws-sdk-go/aws/session"
  "github.com/aws/aws-sdk-go/service/dynamodb"
)

type SensorData struct {
  ClientID    string  `json:"clientId"`
  Temperature float64 `json:"temperature"`
  Humidity    float64 `json:"humidity"`
}

func handler(ctx context.Context, event SensorData) error {
  // 1. Create AWS Session
  sess := session.Must(session.NewSession())

  // 2. Connect to DynamoDB
  db := dynamodb.New(sess)

  // 3. Prepare Data for DynamoDB
  input := &dynamodb.PutItemInput{
    TableName: aws.String("ThingDataTable"),
    Item: map[string]*dynamodb.AttributeValue{
      "clientId": {
        S: aws.String(event.ClientID),
      },
      "createdAt": {
        S: aws.String(time.Now().UTC().Format(time.RFC3339)),
      },
      "temperature": {
        N: aws.String(fmt.Sprintf("%.2f", event.Temperature)),
      },
      "humidity": {
        N: aws.String(fmt.Sprintf("%.2f", event.Humidity)),
      },
    },
  }

  // 4. Save to DynamoDB
  _, err := db.PutItem(input)
  return err
}

func main() {
  lambda.Start(handler)
}
