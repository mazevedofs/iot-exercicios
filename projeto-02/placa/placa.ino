#include <UIPEthernet.h>
byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x42 };
EthernetClient ethClient;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#include <LiquidCrystal.h>

// Se não definirmos o nome ONLINE, podemos utilizar uma versão diferente da
// biblioteca que simula MQTT via a comunicação serial
#ifdef ONLINE
#include <PubSubClient.h>
#else
#include <SerialPubSubClient.h>
#endif

// const char* MQTT_SERVER = "test.mosquitto.org";
IPAddress MQTT_SERVER(192, 168, 3, 136);
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "arduino-placa-42";

const char* topic = "vagas/#";

const int QUANTIDADE_VAGAS = 50;
const int vagas[QUANTIDADE_VAGAS] = {};

const int STATUS_OCUPADA = 1;
const int STATUS_LIVRE = 0;
const int STATUS_OFFLINE = -1;

void inicializarVagas() {
	for (int i = 0; i < QUANTIDADE_VAGAS; i++) {
		vagas[i] = STATUS_OFFLINE;
	}
}

int extrairIdVaga(char* topico) {
	int dezena = topico[6] - '0';
	int unidade = topico[7] - '0';

	return dezena * 10 + unidade;
}


void callback(char* topico, byte* mensagem, unsigned int length) {
	tempoUltimaMensagem = millis();

	int idVaga = extrairIdVaga(topico);
	int indice = idVaga - 1;

	if (length == 0) {
		// mensagem vazia, devemos considerar que a vaga não existe
		vagas[indice] = -1;
	}
	int msg = mensagem[0] - '0';

	vagas[indice] = msg;
	atualizarContagem();
}

void atualizarContagem() {
	int ocupadas = 0;
	int livres = 0;

	for(int i = 0; i < QUANTIDADE_VAGAS; i++) {
		switch(vagas[i]) {
			case STATUS_OCUPADA:
			ocupadas++
			break;
			case STATUS_LIVRE:
			livres++;
			break;
			case STATUS_OFFLINE;
			// fazer nada
			break;
		}
	}

	exibirContagem(livres, ocupadas);
}

exibirContagem(int livres, int ocupadas) {
	lcd.clear();
	lcd.print("Livres: ");
	lcd.print(livres);

	lcd.setCursor(0, 1);
	lcd.print("Ocupadas: ");
	lcd.print(ocupadas);
}

PubSubClient client(MQTT_SERVER, callback, MQTT_PORT, ethClient);

void setup() {
	Serial.begin(9600);
	while (!Serial) {};

	lcd.begin(16, 2);

	inicializarVagas();
	#ifdef ONLINE
	configurarEthernet()

	conectarMQTT()
	#endif
}

void configurarEthernet() {
	if(!Ethernet.begin(mac)) {
		Serial.println("Falha no DHCP");
	};
}

int conectarMQTT() {
	return client.connect(MQTT_CLIENT_ID);
}

// Variavel global para guardar o tempo da última tentativa de conexão
unsigned long ultimaTentativaReconectar = 0;
int checkReconectarMQTT() {
	if(!client.connected()) {
		long agora = millis();
		if(agora - ultimaTentativaReconectar > 5000) {
			Serial.println("reconectando...");
			return conectarMQTT();
		}
		ultimaTentativaReconectar = agora;
	}
}

int tempoUltimaMensagemMQTT = 0;
const long TEMPO_DESLIGAR_LED = 10 * 1000;
// Verifica se há necessidade de deligar o LCD
void checkDesligarLCD() {
	long agora = millis();
	if(agora - tempoUltimaMensagemMQTT > TEMPO_DESLIGAR_LED) {
		lcd.noDisplay()
	} else {
		lcd.display();
	}
}

void loop() {
	checkReconectarMQTT();
	// Chamada de método requirido pela biblioteca MQTT
	// ele é necessário para que ela tenha uma oportunidade de processar envios e recebimentos de respostas
	client.loop();

	checkDesligarLCD();
}