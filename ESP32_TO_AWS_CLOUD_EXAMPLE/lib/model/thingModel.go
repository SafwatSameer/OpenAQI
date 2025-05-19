package model

type ThingPayload struct {
    ClientId    string  `json:"clientId"`
    Temperature float64 `json:"temperature"`
    Humidity    float64 `json:"humidity"`
}

type ThingData struct {
    ThingPayload
    CreatedAt string `json:"createdAt"`
}
