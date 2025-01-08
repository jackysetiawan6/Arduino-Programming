using M2Mqtt;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.Json;

string iotEndpoint = "a1kwmoq0xfo7wp-ats.iot.us-east-1.amazonaws.com";
int brokerPort = 8883;
string topic = "sensor_group_03";

var caCert = X509Certificate.CreateFromCertFile(Path.Join(AppContext.BaseDirectory, "resources\\AmazonRootCA1.pem"));
var clientCert = new X509Certificate2(Path.Join(AppContext.BaseDirectory, "resources\\certificate.pfx"), "newCertificate");
var client = new MqttClient(iotEndpoint, brokerPort, true, caCert, clientCert, MqttSslProtocols.TLSv1_2);

string clientId = Guid.NewGuid().ToString();
client.Connect(clientId);

Console.WriteLine($"Connected to AWS IoT with client id: {clientId}.");

Random random = new();

while (true)
{
    var payload = new
    {
        Light = random.Next(0, 1025),
        Temperature = (random.NextDouble() * (50 - 10) + 10).ToString("F1"),
        Humidity = random.Next(30, 100).ToString(),
        LED_State = random.Next(0, 2),
        LED_Override = random.Next(-1, 2),
    };
    string jsonPayload = JsonSerializer.Serialize(payload);
    Console.WriteLine($"Payload: {jsonPayload}");
    client.Publish(topic, Encoding.UTF8.GetBytes($"{jsonPayload}"));
    Thread.Sleep(1500);
}