package helper

import (
    "encoding/json"
    "log"
)

func EventLogger(event interface{}) {
    eventJSON, err := json.Marshal(event)
    if err != nil {
        log.Printf("Error marshalling event: %v", err)
    }
    log.Printf("Event:\n%s", string(eventJSON))
}