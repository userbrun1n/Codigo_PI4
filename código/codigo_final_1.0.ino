// Importação das bibliotecas do sensor acelerômetro
#include <Wire.h>
#include <MPU6050.h>
#include <SD.h>

// Criando um objeto mpu para o sensor
MPU6050 mpu;

// Variáveis de offset
int16_t ax_offset, ay_offset, az_offset;
int16_t gx_offset, gy_offset, gz_offset;

// Variáveis para armazenar a leitura anterior do sensor
int16_t ax_prev, ay_prev, az_prev;
int16_t gx_prev, gy_prev, gz_prev;

// Pino de seleção do chip do módulo SD
const int chipSelect = 10;

// Variável para controlar a primeira leitura
bool primeiraLeitura = true;

// Inicializando todos os componentes
void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  
  // Inicializar comunicação com o cartão SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Falha ao inicializar o cartão SD!");
    return;
  }
  Serial.println("Cartão SD inicializado com sucesso!");

  // Verificando a inicialização do sensor e acusando falhas, se essas ocorrerem.
  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado com sucesso!");
  } else {
    Serial.println("Falha na conexão com o MPU6050.");
  }

  // Diminuir a sensibilidade do acelerômetro manualmente
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8); // Configurar para ±8g

  // Calibração Inicial
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Definir offsets (esses valores podem precisar de ajustes)
  ax_offset = ax;
  ay_offset = ay;
  az_offset = az - 16384; // Subtrair o valor da gravidade
  gx_offset = gx;
  gy_offset = gy;
  gz_offset = gz;

  // Armazenar as leituras iniciais
  ax_prev = ax;
  ay_prev = ay;
  az_prev = az;
  gx_prev = gx;
  gy_prev = gy;
  gz_prev = gz;
}

// Loop para a análise do movimento da caixa e declaração das variáveis nos 3 eixos dimensionais
void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  // O sensor armazena as variações
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Aplicar offsets
  ax -= ax_offset;
  ay -= ay_offset;
  az -= az_offset;
  gx -= gx_offset;
  gy -= gy_offset;
  gz -= gz_offset;
  
  // Calcular a diferença nas leituras do sensor
  float deltaAcelX = abs(ax - ax_prev);
  float deltaAcelY = abs(ay - ay_prev);
  float deltaAcelZ = abs(az - az_prev);

  // Atualizar as leituras anteriores
  ax_prev = ax;
  ay_prev = ay;
  az_prev = az;
  gx_prev = gx;
  gy_prev = gy;
  gz_prev = gz;

  // Verificar se houve uma mudança significativa na aceleração
  if (deltaAcelX > 10000 || deltaAcelY > 10000 || deltaAcelZ > 9000) { // Ajustar este valor conforme necessário para detectar quedas significativas
    Serial.println("Queda significativa detectada!");

    if (!primeiraLeitura) {
      logData(ax, ay, az, gx, gy, gz); // Mostrar os dados no Monitor Serial.
      saveDataToSD(ax, ay, az, gx, gy, gz); // Salvar os dados no cartão SD
    }

    // Desativar o flag da primeira leitura
    primeiraLeitura = false;
  }
  
  delay(500);
}

void logData(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz) {
  // Mostrar os dados no Monitor Serial
  Serial.println("O equipamento sofreu uma queda significativa.");
  
  // Valores de aceleração
  Serial.print("Aceleração:\tX (esquerda-direita): "); Serial.print(ax);
  Serial.print("\tY (frente-trás): "); Serial.print(ay);
  Serial.print("\tZ (cima-baixo): "); Serial.println(az);
  
  // Valores do giroscópio
  Serial.print("Giroscópio:\tX (rotação frente-trás): "); Serial.print(gx);
  Serial.print("\tY (rotação esquerda-direita): "); Serial.print(gy);
  Serial.print("\tZ (rotação horário-anti-horário): "); Serial.println(gz);

  Serial.println("Dados registrados.");
}

void saveDataToSD(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz) {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
  if (dataFile) {
    dataFile.println("O equipamento sofreu uma queda significativa.");
    
    // Valores de aceleração
    dataFile.print("Aceleração:\tX (esquerda-direita): "); dataFile.print(ax);
    dataFile.print("\tY (frente-trás): "); dataFile.print(ay);
    dataFile.print("\tZ (cima-baixo): "); dataFile.println(az);
    
    // Valores do giroscópio
    dataFile.print("Giroscópio:\tX (rotação frente-trás): "); dataFile.print(gx);
    dataFile.print("\tY (rotação esquerda-direita): "); dataFile.print(gy);
    dataFile.print("\tZ (rotação horário-anti-horário): "); dataFile.println(gz);

    dataFile.println("Dados registrados.");
    dataFile.close();
    Serial.println("Dados salvos no cartão SD.");
  } else {
    Serial.println("Erro ao abrir o arquivo datalog.txt para escrita.");
  }
}
