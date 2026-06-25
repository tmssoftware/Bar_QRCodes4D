type
  TQRCodeErrorCorrection = (qrecL, qrecM, qrecQ, qrecH);

  TQRCodeMatrix = array of array of Integer;
  TIntegerArray = array of Integer;

  TTMSFNCQRCode = class(TTMSFNCControl)
  private
    FText: string;
    FDarkColor: TTMSFNCGraphicsColor;
    FLightColor: TTMSFNCGraphicsColor;
    FQuietZone: Integer;
    FErrorCorrection: TQRCodeErrorCorrection;
    FModules: TQRCodeMatrix;
    FModuleCount: Integer;
    FDirty: Boolean;
    procedure SetText(const AValue: string);
    procedure SetDarkColor(AValue: TTMSFNCGraphicsColor);
    procedure SetLightColor(AValue: TTMSFNCGraphicsColor);
    procedure SetQuietZone(AValue: Integer);
    procedure SetErrorCorrection(AValue: TQRCodeErrorCorrection);
    procedure BuildQRCode;
    procedure DrawQRCode(AGraphics: TTMSFNCGraphics; const ARect: TRectF);
    procedure RenderToGraphics(AGraphics: TTMSFNCGraphics; AWidth, AHeight: Integer);
  protected
    procedure Draw(AGraphics: TTMSFNCGraphics; ARect: TRectF); override;
    procedure RegisterRuntimeClasses; override;
  public
    constructor Create(AOwner: TComponent); override;
    procedure Resize; override;
    procedure SaveToPNG(const AFileName: string);
    procedure SaveToSVG(const AFileName: string);
    property ModuleCount: Integer read FModuleCount;
  published
    property Text: string read FText write SetText;
    property DarkColor: TTMSFNCGraphicsColor read FDarkColor write SetDarkColor default gcBlack;
    property LightColor: TTMSFNCGraphicsColor read FLightColor write SetLightColor default gcWhite;
    property QuietZone: Integer read FQuietZone write SetQuietZone default 4;
    property ErrorCorrection: TQRCodeErrorCorrection read FErrorCorrection write SetErrorCorrection default qrecH;
  end;

procedure Register;

implementation

const
  QRMode8BitByte = 4;
  QRPad0 = $EC;
  QRPad1 = $11;

  QRPatternPositionTable: array[1..40] of array[0..6] of Integer = (
    (-1,-1,-1,-1,-1,-1,-1),
    (6,18,-1,-1,-1,-1,-1),
    (6,22,-1,-1,-1,-1,-1),
    (6,26,-1,-1,-1,-1,-1),
    (6,30,-1,-1,-1,-1,-1),
    (6,34,-1,-1,-1,-1,-1),
    (6,22,38,-1,-1,-1,-1),
    (6,24,42,-1,-1,-1,-1),
    (6,26,46,-1,-1,-1,-1),
    (6,28,50,-1,-1,-1,-1),
    (6,30,54,-1,-1,-1,-1),
    (6,32,58,-1,-1,-1,-1),
    (6,34,62,-1,-1,-1,-1),
    (6,26,46,66,-1,-1,-1),
    (6,26,48,70,-1,-1,-1),
    (6,26,50,74,-1,-1,-1),
    (6,30,54,78,-1,-1,-1),
    (6,30,56,82,-1,-1,-1),
    (6,30,58,86,-1,-1,-1),
    (6,34,62,90,-1,-1,-1),
    (6,28,50,72,94,-1,-1),
    (6,26,50,74,98,-1,-1),
    (6,30,54,78,102,-1,-1),
    (6,28,54,80,106,-1,-1),
    (6,32,58,84,110,-1,-1),
    (6,30,58,86,114,-1,-1),
    (6,34,62,90,118,-1,-1),
    (6,26,50,74,98,122,-1),
    (6,30,54,78,102,126,-1),
    (6,26,52,78,104,130,-1),
    (6,30,56,82,108,134,-1),
    (6,34,60,86,112,138,-1),
    (6,30,58,86,114,142,-1),
    (6,34,62,90,118,146,-1),
    (6,30,54,78,102,126,150),
    (6,24,50,76,102,128,154),
    (6,28,54,80,106,132,158),
    (6,32,58,84,110,136,162),
    (6,26,54,82,110,138,166),
    (6,30,58,86,114,142,170)
  );

  QRLimitLength: array[1..40, 0..3] of Integer = (
    (17,14,11,7),(32,26,20,14),(53,42,32,24),(78,62,46,34),
    (106,84,60,44),(134,106,74,58),(154,122,86,64),(192,152,108,84),
    (230,180,130,98),(271,213,151,119),(321,251,177,137),(367,287,203,155),
    (425,331,241,177),(458,362,258,194),(520,412,292,220),(586,450,322,250),
    (644,504,364,280),(718,560,394,310),(792,624,442,338),(858,666,482,382),
    (929,711,509,403),(1003,779,565,439),(1091,857,611,461),(1171,911,661,511),
    (1273,997,715,535),(1367,1059,751,593),(1465,1125,805,625),(1528,1190,868,658),
    (1628,1264,908,698),(1732,1370,982,742),(1840,1452,1030,790),(1952,1538,1112,842),
    (2068,1628,1168,898),(2188,1722,1228,958),(2303,1809,1283,983),(2431,1911,1351,1051),
    (2563,1989,1423,1093),(2699,2099,1499,1139),(2809,2213,1579,1219),(2953,2331,1663,1273)
  );

  // Values are grouped as count, totalCount, dataCount. Four rows per QR version: L, M, Q, H.
  QRRSBlockTable: array[0..159] of string = (
    '1,26,19','1,26,16','1,26,13','1,26,9',
    '1,44,34','1,44,28','1,44,22','1,44,16',
    '1,70,55','1,70,44','2,35,17','2,35,13',
    '1,100,80','2,50,32','2,50,24','4,25,9',
    '1,134,108','2,67,43','2,33,15,2,34,16','2,33,11,2,34,12',
    '2,86,68','4,43,27','4,43,19','4,43,15',
    '2,98,78','4,49,31','2,32,14,4,33,15','4,39,13,1,40,14',
    '2,121,97','2,60,38,2,61,39','4,40,18,2,41,19','4,40,14,2,41,15',
    '2,146,116','3,58,36,2,59,37','4,36,16,4,37,17','4,36,12,4,37,13',
    '2,86,68,2,87,69','4,69,43,1,70,44','6,43,19,2,44,20','6,43,15,2,44,16',
    '4,101,81','1,80,50,4,81,51','4,50,22,4,51,23','3,36,12,8,37,13',
    '2,116,92,2,117,93','6,58,36,2,59,37','4,46,20,6,47,21','7,42,14,4,43,15',
    '4,133,107','8,59,37,1,60,38','8,44,20,4,45,21','12,33,11,4,34,12',
    '3,145,115,1,146,116','4,64,40,5,65,41','11,36,16,5,37,17','11,36,12,5,37,13',
    '5,109,87,1,110,88','5,65,41,5,66,42','5,54,24,7,55,25','11,36,12',
    '5,122,98,1,123,99','7,73,45,3,74,46','15,43,19,2,44,20','3,45,15,13,46,16',
    '1,135,107,5,136,108','10,74,46,1,75,47','1,50,22,15,51,23','2,42,14,17,43,15',
    '5,150,120,1,151,121','9,69,43,4,70,44','17,50,22,1,51,23','2,42,14,19,43,15',
    '3,141,113,4,142,114','3,70,44,11,71,45','17,47,21,4,48,22','9,39,13,16,40,14',
    '3,135,107,5,136,108','3,67,41,13,68,42','15,54,24,5,55,25','15,43,15,10,44,16',
    '4,144,116,4,145,117','17,68,42','17,50,22,6,51,23','19,46,16,6,47,17',
    '2,139,111,7,140,112','17,74,46','7,54,24,16,55,25','34,37,13',
    '4,151,121,5,152,122','4,75,47,14,76,48','11,54,24,14,55,25','16,45,15,14,46,16',
    '6,147,117,4,148,118','6,73,45,14,74,46','11,54,24,16,55,25','30,46,16,2,47,17',
    '8,132,106,4,133,107','8,75,47,13,76,48','7,54,24,22,55,25','22,45,15,13,46,16',
    '10,142,114,2,143,115','19,74,46,4,75,47','28,50,22,6,51,23','33,46,16,4,47,17',
    '8,152,122,4,153,123','22,73,45,3,74,46','8,53,23,26,54,24','12,45,15,28,46,16',
    '3,147,117,10,148,118','3,73,45,23,74,46','4,54,24,31,55,25','11,45,15,31,46,16',
    '7,146,116,7,147,117','21,73,45,7,74,46','1,53,23,37,54,24','19,45,15,26,46,16',
    '5,145,115,10,146,116','19,75,47,10,76,48','15,54,24,25,55,25','23,45,15,25,46,16',
    '13,145,115,3,146,116','2,74,46,29,75,47','42,54,24,1,55,25','23,45,15,28,46,16',
    '17,145,115','10,74,46,23,75,47','10,54,24,35,55,25','19,45,15,35,46,16',
    '17,145,115,1,146,116','14,74,46,21,75,47','29,54,24,19,55,25','11,45,15,46,46,16',
    '13,145,115,6,146,116','14,74,46,23,75,47','44,54,24,7,55,25','59,46,16,1,47,17',
    '12,151,121,7,152,122','12,75,47,26,76,48','39,54,24,14,55,25','22,45,15,41,46,16',
    '6,151,121,14,152,122','6,75,47,34,76,48','46,54,24,10,55,25','2,45,15,64,46,16',
    '17,152,122,4,153,123','29,74,46,14,75,47','49,54,24,10,55,25','24,45,15,46,46,16',
    '4,152,122,18,153,123','13,74,46,32,75,47','48,54,24,14,55,25','42,45,15,32,46,16',
    '20,147,117,4,148,118','40,75,47,7,76,48','43,54,24,22,55,25','10,45,15,67,46,16',
    '19,148,118,6,149,119','18,75,47,31,76,48','34,54,24,34,55,25','20,45,15,61,46,16'
  );

type
  TRSBlock = record
    TotalCount: Integer;
    DataCount: Integer;
  end;
  TRSBlocks = array of TRSBlock;

  TQRPolynomial = record
    Num: TIntegerArray;
  end;

  TQRBitBuffer = class
  private
    FBuffer: TBytes;
    FLength: Integer;
  public
    procedure Put(ANum, ALength: Integer);
    procedure PutBit(ABit: Boolean);
    function GetLengthInBits: Integer;
    function ByteAt(AIndex: Integer): Byte;
  end;

var
  GExpTable: array[0..255] of Integer;
  GLogTable: array[0..255] of Integer;
  GTablesReady: Boolean = False;

function ErrorLevelValue(AValue: TQRCodeErrorCorrection): Integer;
begin
  case AValue of
    qrecL: Result := 1;
    qrecM: Result := 0;
    qrecQ: Result := 3;
  else
    Result := 2;
  end;
end;

function ErrorLevelIndex(AValue: TQRCodeErrorCorrection): Integer;
begin
  case AValue of
    qrecL: Result := 0;
    qrecM: Result := 1;
    qrecQ: Result := 2;
  else
    Result := 3;
  end;
end;

function UTF8Length(const S: string): Integer;
begin
  Result := Length(TEncoding.UTF8.GetBytes(S));
end;

function GetTypeNumber(const S: string; ALevel: TQRCodeErrorCorrection): Integer;
var
  L, I, Index: Integer;
begin
  L := UTF8Length(S);
  Index := ErrorLevelIndex(ALevel);
  for I := 1 to 40 do
    if L <= QRLimitLength[I, Index] then
      Exit(I);
  raise EInvalidOperation.Create('Too long data');
end;

procedure EnsureGaloisTables;
var
  I: Integer;
begin
  if GTablesReady then
    Exit;

  for I := 0 to 7 do
    GExpTable[I] := 1 shl I;
  for I := 8 to 255 do
    GExpTable[I] := GExpTable[I - 4] xor GExpTable[I - 5] xor GExpTable[I - 6] xor GExpTable[I - 8];
  {$IFNDEF WEBLIB}
  FillChar(GLogTable, SizeOf(GLogTable), 0);
  {$ENDIF}
  for I := 0 to 254 do
    GLogTable[GExpTable[I]] := I;

  GTablesReady := True;
end;

function GExp(N: Integer): Integer;
begin
  EnsureGaloisTables;
  while N < 0 do Inc(N, 255);
  while N >= 256 do Dec(N, 255);
  Result := GExpTable[N];
end;

function GLog(N: Integer): Integer;
begin
  EnsureGaloisTables;
  if N < 1 then
    raise EInvalidOperation.CreateFmt('glog(%d)', [N]);
  Result := GLogTable[N];
end;

function PolyCreate(const ANum: array of Integer; AShift: Integer): TQRPolynomial;
var
  Offset, I, Count: Integer;
  Num: TIntegerArray;
begin
  Offset := 0;
  while (Offset < Length(ANum)) and (ANum[Offset] = 0) do
    Inc(Offset);
  Count := Length(ANum) - Offset;
  SetLength(Num, Count + AShift);
  for I := 0 to Count - 1 do
    Num[I] := ANum[I + Offset];
  Result.Num := Num;
end;

function PolyCreateFromArray(const ANum: TIntegerArray; AShift: Integer): TQRPolynomial;
var
  Offset, I, Count: Integer;
  Num: TIntegerArray;
begin
  Offset := 0;
  while (Offset < Length(ANum)) and (ANum[Offset] = 0) do
    Inc(Offset);
  Count := Length(ANum) - Offset;
  SetLength(Num, Count + AShift);
  for I := 0 to Count - 1 do
    Num[I] := ANum[I + Offset];
  Result.Num := Num;
end;

function PolyMultiply(const A, B: TQRPolynomial): TQRPolynomial;
var
  I, J: Integer;
  Num: TIntegerArray;
begin
  if (Length(A.Num) = 0) or (Length(B.Num) = 0) then
  begin
    SetLength(Result.Num, 0);
    Exit;
  end;

  SetLength(Num, Length(A.Num) + Length(B.Num) - 1);
  for I := 0 to Length(A.Num) - 1 do
    for J := 0 to Length(B.Num) - 1 do
      if (A.Num[I] <> 0) and (B.Num[J] <> 0) then
        Num[I + J] := Num[I + J] xor GExp(GLog(A.Num[I]) + GLog(B.Num[J]));
  Result.Num := Num;
end;

function PolyMod(const A, B: TQRPolynomial): TQRPolynomial;
var
  Ratio, I: Integer;
  Num: TIntegerArray;
begin
  if Length(A.Num) = 0 then
    Exit(A);
  if Length(B.Num) = 0 then
    raise EInvalidOperation.Create('Invalid polynomial divisor');
  if A.Num[0] = 0 then
    Exit(PolyMod(PolyCreateFromArray(A.Num, 0), B));
  if B.Num[0] = 0 then
    raise EInvalidOperation.Create('Invalid polynomial divisor');

  if Length(A.Num) - Length(B.Num) < 0 then
    Exit(A);

  Ratio := GLog(A.Num[0]) - GLog(B.Num[0]);
  SetLength(Num, Length(A.Num));
  for I := 0 to Length(A.Num) - 1 do
    Num[I] := A.Num[I];
  for I := 0 to Length(B.Num) - 1 do
    if B.Num[I] <> 0 then
      Num[I] := Num[I] xor GExp(GLog(B.Num[I]) + Ratio);
  Result := PolyMod(PolyCreateFromArray(Num, 0), B);
end;

function GetErrorCorrectPolynomial(AErrorCorrectLength: Integer): TQRPolynomial;
var
  I: Integer;
begin
  Result := PolyCreate([1], 0);
  for I := 0 to AErrorCorrectLength - 1 do
    Result := PolyMultiply(Result, PolyCreate([1, GExp(I)], 0));
end;

function BCHDigit(AData: Integer): Integer;
begin
  Result := 0;
  while AData <> 0 do
  begin
    Inc(Result);
    AData := AData shr 1;
  end;
end;

function BCHTypeInfo(AData: Integer): Integer;
const
  G15 = (1 shl 10) or (1 shl 8) or (1 shl 5) or (1 shl 4) or (1 shl 2) or (1 shl 1) or (1 shl 0);
  G15Mask = (1 shl 14) or (1 shl 12) or (1 shl 10) or (1 shl 4) or (1 shl 1);
var
  D: Integer;
begin
  D := AData shl 10;
  while BCHDigit(D) - BCHDigit(G15) >= 0 do
    D := D xor (G15 shl (BCHDigit(D) - BCHDigit(G15)));
  Result := ((AData shl 10) or D) xor G15Mask;
end;

function BCHTypeNumber(AData: Integer): Integer;
const
  G18 = (1 shl 12) or (1 shl 11) or (1 shl 10) or (1 shl 9) or (1 shl 8) or (1 shl 5) or (1 shl 2) or (1 shl 0);
var
  D: Integer;
begin
  D := AData shl 12;
  while BCHDigit(D) - BCHDigit(G18) >= 0 do
    D := D xor (G18 shl (BCHDigit(D) - BCHDigit(G18)));
  Result := (AData shl 12) or D;
end;

function LengthInBits(ATypeNumber: Integer): Integer;
begin
  if ATypeNumber < 10 then
    Result := 8
  else
    Result := 16;
end;

function Mask(AMaskPattern, I, J: Integer): Boolean;
begin
  case AMaskPattern of
    0: Result := ((I + J) mod 2) = 0;
    1: Result := (I mod 2) = 0;
    2: Result := (J mod 3) = 0;
    3: Result := ((I + J) mod 3) = 0;
    4: Result := ((I div 2 + J div 3) mod 2) = 0;
    5: Result := (((I * J) mod 2) + ((I * J) mod 3)) = 0;
    6: Result := ((((I * J) mod 2) + ((I * J) mod 3)) mod 2) = 0;
    7: Result := ((((I * J) mod 3) + ((I + J) mod 2)) mod 2) = 0;
  else
    raise EInvalidOperation.Create('Bad mask pattern');
  end;
end;

function ParseCSVIntegers(const S: string): TIntegerArray;
var
  Parts: TStringList;
  I: Integer;
begin
  Parts := TStringList.Create;
  try
    Parts.StrictDelimiter := True;
    Parts.Delimiter := ',';
    Parts.DelimitedText := S;
    SetLength(Result, Parts.Count);
    for I := 0 to Parts.Count - 1 do
      Result[I] := StrToInt(Trim(Parts[I]));
  finally
    Parts.Free;
  end;
end;

function GetRSBlocks(ATypeNumber: Integer; ALevel: TQRCodeErrorCorrection): TRSBlocks;
var
  Values: TIntegerArray;
  I, J, N, Count, TotalCount, DataCount: Integer;
begin
  Values := ParseCSVIntegers(QRRSBlockTable[(ATypeNumber - 1) * 4 + ErrorLevelIndex(ALevel)]);
  SetLength(Result, 0);
  I := 0;
  while I < Length(Values) do
  begin
    Count := Values[I];
    TotalCount := Values[I + 1];
    DataCount := Values[I + 2];
    for J := 0 to Count - 1 do
    begin
      N := Length(Result);
      SetLength(Result, N + 1);
      Result[N].TotalCount := TotalCount;
      Result[N].DataCount := DataCount;
    end;
    Inc(I, 3);
  end;
end;

procedure TQRBitBuffer.Put(ANum, ALength: Integer);
var
  I: Integer;
begin
  for I := 0 to ALength - 1 do
    PutBit(((ANum shr (ALength - I - 1)) and 1) = 1);
end;

procedure TQRBitBuffer.PutBit(ABit: Boolean);
var
  BufIndex: Integer;
begin
  BufIndex := FLength div 8;
  if Length(FBuffer) <= BufIndex then
    SetLength(FBuffer, BufIndex + 1);
  if ABit then
    FBuffer[BufIndex] := FBuffer[BufIndex] or Byte($80 shr (FLength mod 8));
  Inc(FLength);
end;

function TQRBitBuffer.GetLengthInBits: Integer;
begin
  Result := FLength;
end;

function TQRBitBuffer.ByteAt(AIndex: Integer): Byte;
begin
  Result := FBuffer[AIndex];
end;

function CreateBytes(ABuffer: TQRBitBuffer; const ARSBlocks: TRSBlocks): TBytes;
var
  Offset, MaxDcCount, MaxEcCount, R, I, DcCount, EcCount, ModIndex, TotalCodeCount, Index: Integer;
  DCData, ECData: array of TIntegerArray;
  RSPoly, RawPoly, ModPoly: TQRPolynomial;
begin
  Offset := 0;
  MaxDcCount := 0;
  MaxEcCount := 0;
  SetLength(DCData, Length(ARSBlocks));
  SetLength(ECData, Length(ARSBlocks));

  for R := 0 to Length(ARSBlocks) - 1 do
  begin
    DcCount := ARSBlocks[R].DataCount;
    EcCount := ARSBlocks[R].TotalCount - DcCount;
    MaxDcCount := Max(MaxDcCount, DcCount);
    MaxEcCount := Max(MaxEcCount, EcCount);

    SetLength(DCData[R], DcCount);
    for I := 0 to DcCount - 1 do
      DCData[R][I] := ABuffer.ByteAt(I + Offset) and $FF;
    Inc(Offset, DcCount);

    RSPoly := GetErrorCorrectPolynomial(EcCount);
    RawPoly := PolyCreateFromArray(DCData[R], Length(RSPoly.Num) - 1);
    ModPoly := PolyMod(RawPoly, RSPoly);
    SetLength(ECData[R], Length(RSPoly.Num) - 1);
    for I := 0 to Length(ECData[R]) - 1 do
    begin
      ModIndex := I + Length(ModPoly.Num) - Length(ECData[R]);
      if ModIndex >= 0 then
        ECData[R][I] := ModPoly.Num[ModIndex]
      else
        ECData[R][I] := 0;
    end;
  end;

  TotalCodeCount := 0;
  for I := 0 to Length(ARSBlocks) - 1 do
    Inc(TotalCodeCount, ARSBlocks[I].TotalCount);
  SetLength(Result, TotalCodeCount);
  Index := 0;

  for I := 0 to MaxDcCount - 1 do
    for R := 0 to Length(ARSBlocks) - 1 do
      if I < Length(DCData[R]) then
      begin
        Result[Index] := Byte(DCData[R][I]);
        Inc(Index);
      end;

  for I := 0 to MaxEcCount - 1 do
    for R := 0 to Length(ARSBlocks) - 1 do
      if I < Length(ECData[R]) then
      begin
        Result[Index] := Byte(ECData[R][I]);
        Inc(Index);
      end;
end;

function CreateData(ATypeNumber: Integer; ALevel: TQRCodeErrorCorrection; const AData: TBytes): TBytes;
var
  RSBlocks: TRSBlocks;
  Buffer: TQRBitBuffer;
  TotalDataCount, I: Integer;
begin
  RSBlocks := GetRSBlocks(ATypeNumber, ALevel);
  Buffer := TQRBitBuffer.Create;
  try
    Buffer.Put(QRMode8BitByte, 4);
    Buffer.Put(Length(AData), LengthInBits(ATypeNumber));
    for I := 0 to Length(AData) - 1 do
      Buffer.Put(AData[I], 8);

    TotalDataCount := 0;
    for I := 0 to Length(RSBlocks) - 1 do
      Inc(TotalDataCount, RSBlocks[I].DataCount);

    if Buffer.GetLengthInBits > TotalDataCount * 8 then
      raise EInvalidOperation.CreateFmt('Code length overflow. (%d > %d)', [Buffer.GetLengthInBits, TotalDataCount * 8]);

    if Buffer.GetLengthInBits + 4 <= TotalDataCount * 8 then
      Buffer.Put(0, 4);
    while (Buffer.GetLengthInBits mod 8) <> 0 do
      Buffer.PutBit(False);
    while Buffer.GetLengthInBits < TotalDataCount * 8 do
    begin
      Buffer.Put(QRPad0, 8);
      if Buffer.GetLengthInBits >= TotalDataCount * 8 then
        Break;
      Buffer.Put(QRPad1, 8);
    end;

    Result := CreateBytes(Buffer, RSBlocks);
  finally
    Buffer.Free;
  end;
end;

procedure InitMatrix(var AMatrix: TQRCodeMatrix; ASize: Integer);
var
  R, C: Integer;
begin
  SetLength(AMatrix, ASize, ASize);
  for R := 0 to ASize - 1 do
    for C := 0 to ASize - 1 do
      AMatrix[R][C] := -1;
end;

procedure SetupPositionProbePattern(var AMatrix: TQRCodeMatrix; AModuleCount, ARow, ACol: Integer);
var
  R, C: Integer;
begin
  for R := -1 to 7 do
  begin
    if (ARow + R <= -1) or (AModuleCount <= ARow + R) then Continue;
    for C := -1 to 7 do
    begin
      if (ACol + C <= -1) or (AModuleCount <= ACol + C) then Continue;
      if ((0 <= R) and (R <= 6) and ((C = 0) or (C = 6))) or
         ((0 <= C) and (C <= 6) and ((R = 0) or (R = 6))) or
         ((2 <= R) and (R <= 4) and (2 <= C) and (C <= 4)) then
        AMatrix[ARow + R][ACol + C] := 1
      else
        AMatrix[ARow + R][ACol + C] := 0;
    end;
  end;
end;

procedure SetupTimingPattern(var AMatrix: TQRCodeMatrix; AModuleCount: Integer);
var
  I: Integer;
begin
  for I := 8 to AModuleCount - 9 do
  begin
    if AMatrix[I][6] = -1 then
      AMatrix[I][6] := Ord((I mod 2) = 0);
    if AMatrix[6][I] = -1 then
      AMatrix[6][I] := Ord((I mod 2) = 0);
  end;
end;

procedure SetupPositionAdjustPattern(var AMatrix: TQRCodeMatrix; AModuleCount, ATypeNumber: Integer);
var
  I, J, R, C, Row, Col: Integer;
begin
  for I := 0 to 6 do
  begin
    Row := QRPatternPositionTable[ATypeNumber][I];
    if Row < 0 then Continue;
    for J := 0 to 6 do
    begin
      Col := QRPatternPositionTable[ATypeNumber][J];
      if Col < 0 then Continue;
      if AMatrix[Row][Col] <> -1 then Continue;
      for R := -2 to 2 do
        for C := -2 to 2 do
          if (R = -2) or (R = 2) or (C = -2) or (C = 2) or ((R = 0) and (C = 0)) then
            AMatrix[Row + R][Col + C] := 1
          else
            AMatrix[Row + R][Col + C] := 0;
    end;
  end;
end;

procedure SetupTypeNumber(var AMatrix: TQRCodeMatrix; AModuleCount, ATypeNumber: Integer; ATest: Boolean);
var
  Bits, I, ModValue: Integer;
begin
  Bits := BCHTypeNumber(ATypeNumber);
  for I := 0 to 17 do
  begin
    ModValue := Ord((not ATest) and (((Bits shr I) and 1) = 1));
    AMatrix[I div 3][I mod 3 + AModuleCount - 8 - 3] := ModValue;
    AMatrix[I mod 3 + AModuleCount - 8 - 3][I div 3] := ModValue;
  end;
end;

procedure SetupTypeInfo(var AMatrix: TQRCodeMatrix; AModuleCount, AErrorLevelValue, AMaskPattern: Integer; ATest: Boolean);
var
  Data, Bits, I, ModValue: Integer;
begin
  Data := (AErrorLevelValue shl 3) or AMaskPattern;
  Bits := BCHTypeInfo(Data);
  for I := 0 to 14 do
  begin
    ModValue := Ord((not ATest) and (((Bits shr I) and 1) = 1));
    if I < 6 then
      AMatrix[I][8] := ModValue
    else if I < 8 then
      AMatrix[I + 1][8] := ModValue
    else
      AMatrix[AModuleCount - 15 + I][8] := ModValue;

    if I < 8 then
      AMatrix[8][AModuleCount - I - 1] := ModValue
    else if I < 9 then
      AMatrix[8][15 - I] := ModValue
    else
      AMatrix[8][15 - I - 1] := ModValue;
  end;
  AMatrix[AModuleCount - 8][8] := Ord(not ATest);
end;

procedure MapData(var AMatrix: TQRCodeMatrix; AModuleCount: Integer; const AData: TBytes; AMaskPattern: Integer);
var
  IncDir, Row, Col, C, BitIndex, ByteIndex: Integer;
  Dark: Boolean;
begin
  IncDir := -1;
  Row := AModuleCount - 1;
  BitIndex := 7;
  ByteIndex := 0;
  Col := AModuleCount - 1;
  while Col > 0 do
  begin
    if Col = 6 then Dec(Col);
    while True do
    begin
      for C := 0 to 1 do
      begin
        if AMatrix[Row][Col - C] = -1 then
        begin
          Dark := False;
          if ByteIndex < Length(AData) then
            Dark := (((AData[ByteIndex] shr BitIndex) and 1) = 1);
          if Mask(AMaskPattern, Row, Col - C) then
            Dark := not Dark;
          AMatrix[Row][Col - C] := Ord(Dark);
          Dec(BitIndex);
          if BitIndex = -1 then
          begin
            Inc(ByteIndex);
            BitIndex := 7;
          end;
        end;
      end;
      Inc(Row, IncDir);
      if (Row < 0) or (AModuleCount <= Row) then
      begin
        Dec(Row, IncDir);
        IncDir := -IncDir;
        Break;
      end;
    end;
    Dec(Col, 2);
  end;
end;

function LostPoint(const AMatrix: TQRCodeMatrix; AModuleCount: Integer): Integer;
var
  Row, Col, R, C, SameCount, Count, DarkCount: Integer;
  Dark: Boolean;
  Ratio: Double;
begin
  Result := 0;
  for Row := 0 to AModuleCount - 1 do
    for Col := 0 to AModuleCount - 1 do
    begin
      SameCount := 0;
      Dark := AMatrix[Row][Col] = 1;
      for R := -1 to 1 do
      begin
        if (Row + R < 0) or (AModuleCount <= Row + R) then Continue;
        for C := -1 to 1 do
        begin
          if (Col + C < 0) or (AModuleCount <= Col + C) then Continue;
          if (R = 0) and (C = 0) then Continue;
          if Dark = (AMatrix[Row + R][Col + C] = 1) then Inc(SameCount);
        end;
      end;
      if SameCount > 5 then Inc(Result, 3 + SameCount - 5);
    end;

  for Row := 0 to AModuleCount - 2 do
    for Col := 0 to AModuleCount - 2 do
    begin
      Count := Ord(AMatrix[Row][Col] = 1) + Ord(AMatrix[Row + 1][Col] = 1) +
               Ord(AMatrix[Row][Col + 1] = 1) + Ord(AMatrix[Row + 1][Col + 1] = 1);
      if (Count = 0) or (Count = 4) then Inc(Result, 3);
    end;

  for Row := 0 to AModuleCount - 1 do
    for Col := 0 to AModuleCount - 7 do
      if (AMatrix[Row][Col] = 1) and (AMatrix[Row][Col+1] = 0) and
         (AMatrix[Row][Col+2] = 1) and (AMatrix[Row][Col+3] = 1) and
         (AMatrix[Row][Col+4] = 1) and (AMatrix[Row][Col+5] = 0) and
         (AMatrix[Row][Col+6] = 1) then Inc(Result, 40);

  for Col := 0 to AModuleCount - 1 do
    for Row := 0 to AModuleCount - 7 do
      if (AMatrix[Row][Col] = 1) and (AMatrix[Row+1][Col] = 0) and
         (AMatrix[Row+2][Col] = 1) and (AMatrix[Row+3][Col] = 1) and
         (AMatrix[Row+4][Col] = 1) and (AMatrix[Row+5][Col] = 0) and
         (AMatrix[Row+6][Col] = 1) then Inc(Result, 40);

  DarkCount := 0;
  for Row := 0 to AModuleCount - 1 do
    for Col := 0 to AModuleCount - 1 do
      if AMatrix[Row][Col] = 1 then Inc(DarkCount);
  Ratio := Abs(100 * DarkCount / AModuleCount / AModuleCount - 50) / 5;
  Inc(Result, Trunc(Ratio * 10));
end;

procedure MakeImpl(var AMatrix: TQRCodeMatrix; ATypeNumber: Integer; ALevel: TQRCodeErrorCorrection;
  const AData: TBytes; AMaskPattern: Integer; ATest: Boolean);
var
  ModuleCount: Integer;
  DataCache: TBytes;
begin
  ModuleCount := ATypeNumber * 4 + 17;
  InitMatrix(AMatrix, ModuleCount);
  SetupPositionProbePattern(AMatrix, ModuleCount, 0, 0);
  SetupPositionProbePattern(AMatrix, ModuleCount, ModuleCount - 7, 0);
  SetupPositionProbePattern(AMatrix, ModuleCount, 0, ModuleCount - 7);
  SetupPositionAdjustPattern(AMatrix, ModuleCount, ATypeNumber);
  SetupTimingPattern(AMatrix, ModuleCount);
  SetupTypeInfo(AMatrix, ModuleCount, ErrorLevelValue(ALevel), AMaskPattern, ATest);
  if ATypeNumber >= 7 then
    SetupTypeNumber(AMatrix, ModuleCount, ATypeNumber, ATest);
  DataCache := CreateData(ATypeNumber, ALevel, AData);
  MapData(AMatrix, ModuleCount, DataCache, AMaskPattern);
end;

function BestMaskPattern(ATypeNumber: Integer; ALevel: TQRCodeErrorCorrection; const AData: TBytes): Integer;
var
  I, Lost, MinLost: Integer;
  M: TQRCodeMatrix;
begin
  Result := 0;
  MinLost := 0;
  for I := 0 to 7 do
  begin
    MakeImpl(M, ATypeNumber, ALevel, AData, I, True);
    Lost := LostPoint(M, ATypeNumber * 4 + 17);
    if (I = 0) or (Lost < MinLost) then
    begin
      MinLost := Lost;
      Result := I;
    end;
  end;
end;



function ExportDimension(AValue: Double; ADefault: Integer): Integer;
begin
  Result := Round(AValue);
  if Result <= 0 then
    Result := ADefault;
end;

procedure FillGraphicsRect(AGraphics: TTMSFNCGraphics; const ARect: TRectF;
  AColor: TTMSFNCGraphicsColor);
begin
  AGraphics.Fill.Kind := gfkSolid;
  AGraphics.Fill.Color := AColor;
  AGraphics.Stroke.Kind := gskNone;
  AGraphics.DrawRectangle(ARect, gcrmNone);
end;

procedure SaveGraphicsBitmapToPNG(ABitmap: TBitmap; const AFileName: string);
{$IFDEF FMXLIB}
begin
  ABitmap.SaveToFile(AFileName);
end;
{$ENDIF}
{$IFDEF WEBLIB}
var
  ctx: JSValue;
begin
  ctx := ABitmap.Canvas.Context;
  asm
    const link = document.createElement("a");
    link.download = AFileName;
    link.href = ctx.canvas.toDataURL("image/png");
    link.click();
  end;
end;
{$ENDIF}
{$IFDEF VCLLIB}
var
  PNG: TPngImage;
begin
  PNG := TPngImage.Create;
  try
    PNG.Assign(ABitmap);
    PNG.SaveToFile(AFileName);
  finally
    PNG.Free;
  end;
end;
{$ENDIF}

function SVGColor(AColor: TTMSFNCGraphicsColor): string;
{$IFDEF FMXLIB}
var
  ColorValue: Cardinal;
begin
  ColorValue := Cardinal(AColor) and $00FFFFFF;
  Result := '#' + IntToHex((ColorValue shr 16) and $FF, 2) +
    IntToHex((ColorValue shr 8) and $FF, 2) + IntToHex(ColorValue and $FF, 2);
end;
{$ELSE}
var
  ColorValue: TTMSFNCGraphicsColor;
begin
  ColorValue := ColorToRGB(AColor);
  Result := '#' + IntToHex(ColorValue and $FF, 2) +
    IntToHex((ColorValue shr 8) and $FF, 2) +
    IntToHex((ColorValue shr 16) and $FF, 2);
end;
{$ENDIF}

{ TTMSFNCQRCode }

constructor TTMSFNCQRCode.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  Width := 128;
  Height := 128;
  FText := 'https://www.tmssoftware.com';
  FDarkColor := gcBlack;
  FLightColor := gcWhite;
  FQuietZone := 4;
  FErrorCorrection := qrecH;
  FDirty := True;
  Color := FLightColor;
end;

procedure TTMSFNCQRCode.BuildQRCode;
var
  TypeNumber, MaskPattern: Integer;
  Data: TBytes;
begin
  if not FDirty then
    Exit;

  Data := TEncoding.UTF8.GetBytes(FText);
  TypeNumber := GetTypeNumber(FText, FErrorCorrection);
  MaskPattern := BestMaskPattern(TypeNumber, FErrorCorrection, Data);
  MakeImpl(FModules, TypeNumber, FErrorCorrection, Data, MaskPattern, False);
  FModuleCount := TypeNumber * 4 + 17;
  FDirty := False;
end;

procedure TTMSFNCQRCode.Draw(AGraphics: TTMSFNCGraphics; ARect: TRectF);
begin
  inherited;
  DrawQRCode(AGraphics, ARect);
end;

procedure TTMSFNCQRCode.DrawQRCode(AGraphics: TTMSFNCGraphics;
  const ARect: TRectF);
var
  TotalModules, ModuleSize: Integer;
  DrawSize, OffsetX, OffsetY, X, Y: Single;
  R, C: Integer;
begin
  BuildQRCode;

  FillGraphicsRect(AGraphics, ARect, FLightColor);

  if (FModuleCount <= 0) or (ARect.Right <= ARect.Left) or
    (ARect.Bottom <= ARect.Top) then
    Exit;

  TotalModules := FModuleCount + FQuietZone * 2;
  ModuleSize := Floor(Min(ARect.Right - ARect.Left, ARect.Bottom - ARect.Top) /
    TotalModules);
  if ModuleSize < 1 then
    ModuleSize := 1;
  DrawSize := ModuleSize * TotalModules;
  OffsetX := ARect.Left + ((ARect.Right - ARect.Left) - DrawSize) / 2;
  OffsetY := ARect.Top + ((ARect.Bottom - ARect.Top) - DrawSize) / 2;

  FillGraphicsRect(AGraphics, RectF(OffsetX, OffsetY, OffsetX + DrawSize,
    OffsetY + DrawSize), FLightColor);

  for R := 0 to FModuleCount - 1 do
    for C := 0 to FModuleCount - 1 do
      if FModules[R][C] = 1 then
      begin
        X := OffsetX + (C + FQuietZone) * ModuleSize;
        Y := OffsetY + (R + FQuietZone) * ModuleSize;
        FillGraphicsRect(AGraphics, RectF(X, Y, X + ModuleSize,
          Y + ModuleSize), FDarkColor);
      end;
end;

procedure TTMSFNCQRCode.RegisterRuntimeClasses;
begin
  inherited;
  RegisterClass(TTMSFNCQRCode);
end;

procedure TTMSFNCQRCode.RenderToGraphics(AGraphics: TTMSFNCGraphics;
  AWidth, AHeight: Integer);
begin
  DrawQRCode(AGraphics, RectF(0, 0, AWidth, AHeight));
end;

procedure TTMSFNCQRCode.Resize;
begin
  inherited;
  Invalidate;
end;

procedure TTMSFNCQRCode.SaveToPNG(const AFileName: string);
var
  ExportHeight, ExportWidth: Integer;
  Graphics: TTMSFNCGraphics;
begin
  ExportWidth := ExportDimension(Width, 128);
  ExportHeight := ExportDimension(Height, 128);

  Graphics := TTMSFNCGraphics.CreateBitmapCanvas(ExportWidth, ExportHeight,
    False, False);
  try
    if Graphics.BeginScene then
    begin
      RenderToGraphics(Graphics, ExportWidth, ExportHeight);
      Graphics.EndScene;
    end;
    SaveGraphicsBitmapToPNG(Graphics.Bitmap, AFileName);
  finally
    Graphics.Free;
  end;
end;

procedure TTMSFNCQRCode.SaveToSVG(const AFileName: string);
var
  SVG: TStringList;
  ImageWidth, ImageHeight: Integer;
  TotalModules, ModuleSize: Integer;
  DrawSize, OffsetX, OffsetY, X, Y: Integer;
  R, C: Integer;
begin
  BuildQRCode;

  ImageWidth := ExportDimension(Width, 128);
  ImageHeight := ExportDimension(Height, 128);
  TotalModules := FModuleCount + FQuietZone * 2;
  ModuleSize := Min(ImageWidth, ImageHeight) div TotalModules;
  if ModuleSize < 1 then
    ModuleSize := 1;
  DrawSize := ModuleSize * TotalModules;
  OffsetX := (ImageWidth - DrawSize) div 2;
  OffsetY := (ImageHeight - DrawSize) div 2;

  SVG := TStringList.Create;
  try
    SVG.Add('<?xml version="1.0" encoding="UTF-8"?>');
    SVG.Add(Format('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">', [ImageWidth, ImageHeight, ImageWidth, ImageHeight]));
    SVG.Add(Format('  <rect width="100%%" height="100%%" fill="%s"/>', [SVGColor(FLightColor)]));
    SVG.Add(Format('  <rect x="%d" y="%d" width="%d" height="%d" fill="%s"/>', [OffsetX, OffsetY, DrawSize, DrawSize, SVGColor(FLightColor)]));
    SVG.Add(Format('  <g fill="%s">', [SVGColor(FDarkColor)]));
    for R := 0 to FModuleCount - 1 do
      for C := 0 to FModuleCount - 1 do
        if FModules[R][C] = 1 then
        begin
          X := OffsetX + (C + FQuietZone) * ModuleSize;
          Y := OffsetY + (R + FQuietZone) * ModuleSize;
          SVG.Add(Format('    <rect x="%d" y="%d" width="%d" height="%d"/>', [X, Y, ModuleSize, ModuleSize]));
        end;
    SVG.Add('  </g>');
    SVG.Add('</svg>');
    SVG.SaveToFile(AFileName {$IFNDEF WEBLIB}, TEncoding.UTF8{$ENDIF});
  finally
    SVG.Free;
  end;
end;

procedure TTMSFNCQRCode.SetDarkColor(AValue: TTMSFNCGraphicsColor);
begin
  if FDarkColor <> AValue then
  begin
    FDarkColor := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCQRCode.SetErrorCorrection(AValue: TQRCodeErrorCorrection);
begin
  if FErrorCorrection <> AValue then
  begin
    FErrorCorrection := AValue;
    FDirty := True;
    Invalidate;
  end;
end;

procedure TTMSFNCQRCode.SetLightColor(AValue: TTMSFNCGraphicsColor);
begin
  if FLightColor <> AValue then
  begin
    FLightColor := AValue;
    Color := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCQRCode.SetQuietZone(AValue: Integer);
begin
  AValue := Max(0, AValue);
  if FQuietZone <> AValue then
  begin
    FQuietZone := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCQRCode.SetText(const AValue: string);
begin
  if FText <> AValue then
  begin
    FText := AValue;
    FDirty := True;
    Invalidate;
  end;
end;

procedure Register;
begin
  {$IFNDEF WEBLIB}
  RegisterComponents('TMS FNC UI', [TTMSFNCQRCode]);
  {$ENDIF}
end;

end.
