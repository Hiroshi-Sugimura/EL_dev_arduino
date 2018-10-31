//////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol
//	Copyright (C) Hiroshi SUGIMURA 2013.09.27
//////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ELOBJ.h>
#include <EL.h>

// オブジェクトを一つだけサポートする。
EL::EL(byte eoj0, byte eoj1, byte eoj2)
{
  _eoj[0] = eoj0;
  _eoj[1] = eoj1;
  _eoj[2] = eoj2;

  _sendPacketSize = 0;
  memset(_sBuffer, 0, EL_BUFFER_SIZE);
  memset(_rBuffer, 0, EL_BUFFER_SIZE);

  _multi[0] = 224;
  _multi[1] = 0;
  _multi[2] = 23;
  _multi[3] = 0;

  _broad[0] = 192;
  _broad[1] = 168;
  _broad[2] = 255;
  _broad[3] = 255;
}

void EL::begin(void)
{
  // udp
  if (udp.begin(ELPORT))
  {
    // Serial.println("Reseiver udp.begin successful.");								// localPort
  }
  else
  {
    Serial.println("Reseiver udp.begin failed."); // localPort
  }

  ip = Ethernet.localIP(); // ip保存
  _broad[0] = ip[0];       // broadcast address update
  _broad[1] = ip[1];       // netmaskを本当は気にしないとだめ
  _broad[2] = ip[2];       // netmask 24(255.255.255.0)想定

  byte *tmp;
  details.SetData(0x80, tmp = new byte[2]{0x01, 0x30}); // power
  delPtr(tmp);
  details.SetData(0x82, tmp = new byte[5]{0x04, 0x01, 0x0a, 0x01, 0x00}); // version 1.01
  delPtr(tmp);
  details.SetData(0x83, tmp = new byte[19]{0x12, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}); //
  delPtr(tmp);
  details.SetData(0x8a, tmp = new byte[4]{0x03, 0x00, 0x00, 0x77}); // maker KAIT
  delPtr(tmp);
  details.SetData(0x9d, tmp = new byte[4]{0x03, 0x02, 0x80, 0xd6}); // inf p map
  delPtr(tmp);
  details.SetData(0x9e, tmp = new byte[2]{0x01, 0x80}); // set p map
  delPtr(tmp);
  details.SetData(0x9f, tmp = new byte[11]{0x0a, 0x09, 0x80, 0x82, 0x83, 0x8a, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7}); // get p map
  delPtr(tmp);
  details.SetData(0xd3, tmp = new byte[4]{0x03, 0x00, 0x00, 0x01}); //
  delPtr(tmp);
  details.SetData(0xd4, tmp = new byte[3]{0x02, 0x00, 0x02}); //
  delPtr(tmp);
  details.SetData(0xd5, tmp = new byte[4]{0x01, _eoj[0], _eoj[1], _eoj[2]}); // obj list
  delPtr(tmp);
  details.SetData(0xd6, tmp = new byte[4]{0x01, _eoj[0], _eoj[1], _eoj[2]}); // obj list
  delPtr(tmp);
  details.SetData(0xd7, tmp = new byte[3]{0x01, _eoj[0], _eoj[1]}); // class list
  delPtr(tmp);

  // ぎゃー　もっと格好良く描けないのか？こうやって書けないんか・・・？
  // details.SetData( 0xd7, byte[] {0x01, eoj0, eoj1} ); // class list
  // きちんとdeleteしないとリークするだろうし。
  details.printAll();

  // 繋がった宣言
  // 立ち上がったINFを飛ばす，まずはプロファイル
  byte s[] = {
      0x10, 0x81, 0x00, 0x00, 0x0e, 0xf0, 0x01, 0x05, 0xff, 0x01, INF, 0x01, 0x80, 0x01, 0x30};
  sendMulti(s, sizeof(s) / sizeof(s[0]));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// details change
void EL::update(const byte epc, byte pdcedt[])
{
  details.SetData(epc, pdcedt); // power
}

byte *EL::at(const byte epc)
{
  return details.GetData(epc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// sender

// ブロードキャストによる送信
void EL::sendBroad(byte sBuffer[], int size)
{
  if (udp.beginPacket(_broad, ELPORT))
  {
    // Serial.println("UDP beginPacket(B) Successful.");
    udp.write(sBuffer, size);
  }

  if (udp.endPacket())
  {
    // Serial.println("UDP endPacket(B) Successful.");
  }
  else
  {
    Serial.println("UDP endPacket(B) failed.");
  }
}

// マルチと見せかけてブロードキャストによる送信
// このようにしておくとArduinoがマルチ対応したときに互換性を保てるハズ
void EL::sendMulti(byte sBuffer[], int size)
{
  sendBroad(sBuffer, size);
}

// OPC一個用のマルチキャスト関数（変なミスが少なくなるはず）
void EL::sendMultiOPC1(const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
  _sBuffer[EHD1] = 0x10;
  _sBuffer[EHD2] = 0x81;
  _sBuffer[TID] = 0x00;
  _sBuffer[TID + 1] = 0x00;
  _sBuffer[SEOJ] = _eoj[0]; // node profile
  _sBuffer[SEOJ + 1] = _eoj[1];
  _sBuffer[SEOJ + 2] = _eoj[2];
  _sBuffer[DEOJ] = deoj[0]; // node profile
  _sBuffer[DEOJ + 1] = deoj[1];
  _sBuffer[DEOJ + 2] = deoj[2];
  _sBuffer[ESV] = esv;
  _sBuffer[OPC] = 0x01;
  _sBuffer[EPC] = epc;
  memcpy(&_sBuffer[PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
  _sendPacketSize = EDT + pdcedt[0];
  sendMulti(_sBuffer, _sendPacketSize);
}

// IP指定による送信
void EL::send(IPAddress toip, byte sBuffer[], int size)
{

  if (udp.beginPacket(toip, ELPORT))
  {
    // Serial.println("UDP beginPacket Successful.");
    udp.write(sBuffer, size);
  }
  else
  {
    Serial.println("UDP beginPacket failed.");
  }

  if (udp.endPacket())
  {
    // Serial.println("UDP endPacket Successful.");
  }
  else
  {
    Serial.println("UDP endPacket failed.");
  }
}

// OPC1指定による送信(SEOJは初期化時に指定したものを使う)
void EL::sendOPC1(const IPAddress toip, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
  _sBuffer[EHD1] = 0x10;
  _sBuffer[EHD2] = 0x81;
  _sBuffer[TID] = 0x00;
  _sBuffer[TID + 1] = 0x00;
  _sBuffer[SEOJ] = _eoj[0]; // node profile
  _sBuffer[SEOJ + 1] = _eoj[1];
  _sBuffer[SEOJ + 2] = _eoj[2];
  _sBuffer[DEOJ] = deoj[0]; // node profile
  _sBuffer[DEOJ + 1] = deoj[1];
  _sBuffer[DEOJ + 2] = deoj[2];
  _sBuffer[ESV] = esv;
  _sBuffer[OPC] = 0x01;
  _sBuffer[EPC] = epc;
  memcpy(&_sBuffer[PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
  _sendPacketSize = EDT + pdcedt[0];
  send(toip, _sBuffer, _sendPacketSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// reseiver
int EL::parsePacket(void)
{
  return udp.parsePacket();
}

IPAddress EL::remoteIP(void)
{
  return udp.remoteIP();
}

int EL::read(void)
{
  _readPacketSize = parsePacket();

  if (_readPacketSize)
  {
    udp.read(_rBuffer, EL_BUFFER_SIZE); // 受け取った内容読み取り
  }
  return _readPacketSize;
}

// details
void EL::returner(void)
{
  ///////////////////////////////////////////////////////////////////
  // 動作状態の変更
  IPAddress remIP = remoteIP();
  byte deoj[] = {_rBuffer[DEOJ], _rBuffer[DEOJ + 1], _rBuffer[DEOJ + 2]};
  const byte esv = _rBuffer[ESV];
  const byte epc = _rBuffer[EPC];
  byte *pdcedt = details.GetData(epc);

  switch (esv)
  {
  case SETI:
    break; // Setは返信しない
  ///////////////////////////////////////////////////////////////////
  // SETC, Get, INF_REQ は返信処理がある
  case SETC:
  case GET:
    Serial.println("get:");
    Serial.println(epc);
    Serial.println(pdcedt[0]);
    Serial.println(pdcedt[1]);

    if (pdcedt)
    { // そのEPCがある場合
      sendOPC1(remIP, deoj, (esv + 0x10), epc, pdcedt);
    }
    else
    {
      pdcedt[0] = 0x00;
      sendOPC1(remIP, deoj, (esv - 0x10), epc, pdcedt);
    }
    break;

  // ユニキャストへの返信ここまで，INFはマルチキャスト
  case INF_REQ:
    if (pdcedt)
    { // そのEPCがある場合、マルチキャスト
      sendMultiOPC1(deoj, (esv + 0x10), epc, pdcedt);
    }
    else
    { // ない場合はエラーなのでユニキャストで返信
      pdcedt[0] = 0x00;
      sendOPC1(remIP, deoj, (esv - 0x10), epc, pdcedt);
    }
    break;
    //  INF_REQここまで

  default: // 解釈不可能なESV
    break;
  }
}
// EL処理ここまで

void EL::printNetData()
{
  char str[20];
  Serial.println("-----------------------------------");

  // IP
  // print your WiFi shield's IP address:
  ip = Ethernet.localIP();
  Serial.print("IP  Address: ");
  Serial.println(ip);

  IPAddress dgwip = Ethernet.gatewayIP();
  Serial.print("DGW Address: ");
  Serial.println(dgwip);

  IPAddress smip = Ethernet.subnetMask();
  Serial.print("SM  Address: ");
  Serial.println(smip);
  Serial.println("-----------------------------------");
}

// byte[] を安全にdeleteする
inline void EL::delPtr(byte ptr[])
{
  if (ptr != nullptr)
  {
    delete[] ptr;
    ptr = nullptr;
  }
}

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
