#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTAsync.h>

void mqttInit(void *arg); // MQTT初始化
int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void onConnectFailure(void *context, MQTTAsync_failureData *response);
void onConnectFailure(void *context, MQTTAsync_failureData *response);
