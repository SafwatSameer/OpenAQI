package service

import (
    "lib/constants"
    "lib/model"
    "os"

    "github.com/aws/aws-sdk-go/aws"
    "github.com/aws/aws-sdk-go/aws/session"
    "github.com/aws/aws-sdk-go/service/dynamodb"
    "github.com/aws/aws-sdk-go/service/dynamodb/dynamodbattribute"
)

type ThingDataDbService struct {
    tableName        string
    dynamoDbProvider *dynamodb.DynamoDB
}

func (s ThingDataDbService) CreateThing(thing model.ThingData) error {
    attributeValue, err := dynamodbattribute.MarshalMap(thing)
    if err != nil {
        return err
    }
    input := &dynamodb.PutItemInput{
        Item:      attributeValue,
        TableName: aws.String(s.tableName),
    }
    _, err = s.dynamoDbProvider.PutItem(input)
    return err
}
func NewThingDataDbService(session *session.Session) ThingDataDbService {
    dynamoDBProvider := dynamodb.New(session)
    return ThingDataDbService{
        tableName:        os.Getenv(constants.THING_TABLE_NAME_KEY),
        dynamoDbProvider: dynamoDBProvider,
    }
}