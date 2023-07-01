extern MQTTAsync client;
extern MQTTAsync_connectOptions conn_opts;

#define MQTT_BROKER_ADDRESS "tcp://xxxxxxx" // MQTT代理的地址
#define MQTT_CLIENT_ID "subscriber"         // MQTT客户端ID
#define MQTT_TOPIC "topic123"               // 要订阅的主题

void mqttInit(void *arg)
{
    // 创建MQTT客户端实例
    MQTTAsync_create(&client, MQTT_BROKER_ADDRESS, MQTT_CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    // 设置消息回调函数
    MQTTAsync_setCallbacks(client, NULL, NULL, messageArrived, NULL);
    // 配置连接选项
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    // 连接到MQTT代理
    MQTTAsync_connect(client, &conn_opts);
    while (1)
    {
        // 保持程序运行
    }
    // 销毁MQTT客户端实例
    MQTTAsync_destroy(&client);
}

// 消息到达回调函数
int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    printf("Received message: %.*s\n", message->payloadlen, (char *)message->payload);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

// 连接成功回调函数
void onConnect(void *context, MQTTAsync_successData *response)
{
    MQTTAsync client = (MQTTAsync)context;

    printf("Connected to MQTT broker\n");
    MQTTAsync_subscribe(client, MQTT_TOPIC, 0, NULL); // 订阅主题
    // 发布主题 一上线
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    char *payload = "online";
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = 0;
    pubmsg.retained = 0;
    opts.onSuccess = NULL;
    opts.onFailure = NULL;
    MQTTAsync_sendMessage(client, MQTT_TOPIC, &pubmsg, &opts);
}

// 连接失败回调函数
void onConnectFailure(void *context, MQTTAsync_failureData *response)
{
    printf("Failed to connect to MQTT broker\n");
}
