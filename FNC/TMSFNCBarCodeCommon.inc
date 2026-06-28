const
{$IF COMPILERVERSION > 28}
  pidWeb = $10000;
{$ELSE}
  pidWeb = $1000;
{$IFEND}
{$IFNDEF PAS2JS}
  TMSWebPlatform = pidWin32 + pidWin64 + pidWeb;
{$ENDIF}

type
  TTMSBarcodeType = (
    tbcCODE39,
    tbcCODE128,
    tbcCODE128A,
    tbcCODE128B,
    tbcCODE128C,
    tbcEAN13,
    tbcEAN8,
    tbcEAN5,
    tbcEAN2,
    tbcUPC,
    tbcUPCE,
    tbcITF14,
    tbcITF,
    tbcMSI,
    tbcMSI10,
    tbcMSI11,
    tbcMSI1010,
    tbcMSI1110,
    tbcPharmacode,
    tbcCodabar,
    tbcCODE93,
    tbcCODE93FullASCII,
    tbcGenericBarcode
  );

{$IFNDEF PAS2JS}
  [ComponentPlatforms(TMSWebPlatform)]
{$ENDIF}
  TTMSFNCBarCode = class(TTMSFNCControl)
  private
    FBackgroundColor: TTMSFNCGraphicsColor;
    FBarcodeType: TTMSBarcodeType;
    FBarColor: TTMSFNCGraphicsColor;
    FBarHeight: Integer;
    FDisplayValue: Boolean;
    FEAN128: Boolean;
    FLastError: string;
    FMargin: Integer;
    FMod43Checksum: Boolean;
    FModuleWidth: Integer;
    FTextMargin: Integer;
    FValue: string;
    {$IFDEF FMXLIB}
    FTextFont: TTMSFNCGraphicsFont;
    procedure FontChanged(Sender: TObject);
    procedure SetFont(const AValue: TTMSFNCGraphicsFont);
    {$ENDIF}
    procedure ApplyTextFont(AGraphics: TTMSFNCGraphics);
    function MeasureTextHeight(const AText: string): Integer;
    procedure SetBackgroundColor(AValue: TTMSFNCGraphicsColor);
    procedure SetBarcodeType(AValue: TTMSBarcodeType);
    procedure SetBarColor(AValue: TTMSFNCGraphicsColor);
    procedure SetBarHeight(AValue: Integer);
    procedure SetDisplayValue(AValue: Boolean);
    procedure SetEAN128(AValue: Boolean);
    procedure SetMargin(AValue: Integer);
    procedure SetMod43Checksum(AValue: Boolean);
    procedure SetModuleWidth(AValue: Integer);
    procedure SetTextMargin(AValue: Integer);
    procedure SetValue(const AValue: string);
  protected
    procedure Draw(AGraphics: TTMSFNCGraphics; ARect: TRectF); override;
    procedure DrawBarcode(AGraphics: TTMSFNCGraphics; const ABounds: TRectF;
      AShowError: Boolean);
    procedure RegisterRuntimeClasses; override;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    function EncodedPattern: string;
    procedure SaveToPNG(const AFileName: string);
    procedure SaveToSVG(const AFileName: string);
    function TryEncode(out APattern, AText: string): Boolean;
    property LastError: string read FLastError;
  published
    {$IFDEF FMXLIB}
    property Font: TTMSFNCGraphicsFont read FTextFont write SetFont;
    {$ENDIF}
    property BackgroundColor: TTMSFNCGraphicsColor read FBackgroundColor write SetBackgroundColor default gcWhite;
    property BarcodeType: TTMSBarcodeType read FBarcodeType write SetBarcodeType default tbcCODE128;
    property BarColor: TTMSFNCGraphicsColor read FBarColor write SetBarColor default gcBlack;
    property BarHeight: Integer read FBarHeight write SetBarHeight default 100;
    property DisplayValue: Boolean read FDisplayValue write SetDisplayValue default True;
    property EAN128: Boolean read FEAN128 write SetEAN128 default False;
    property Margin: Integer read FMargin write SetMargin default 10;
    property Mod43Checksum: Boolean read FMod43Checksum write SetMod43Checksum default False;
    property ModuleWidth: Integer read FModuleWidth write SetModuleWidth default 2;
    property TextMargin: Integer read FTextMargin write SetTextMargin default 2;
    property Value: string read FValue write SetValue;
  end;

function TMSBarcodeTypeName(AType: TTMSBarcodeType): string;
function TMSBarcodeTypeFromName(const AName: string; out AType: TTMSBarcodeType): Boolean;

procedure Register;

implementation

type
  TStringArray = array of string;
  TIntArray = array of Integer;

const
  BarcodeTypeNames: array[TTMSBarcodeType] of string = (
    'CODE39',
    'CODE128',
    'CODE128A',
    'CODE128B',
    'CODE128C',
    'EAN13',
    'EAN8',
    'EAN5',
    'EAN2',
    'UPC',
    'UPCE',
    'ITF14',
    'ITF',
    'MSI',
    'MSI10',
    'MSI11',
    'MSI1010',
    'MSI1110',
    'pharmacode',
    'codabar',
    'CODE93',
    'CODE93FullASCII',
    'GenericBarcode'
  );

  CODE128_SET_A = 0;
  CODE128_SET_B = 1;
  CODE128_SET_C = 2;
  CODE128_START_A = 103;
  CODE128_START_B = 104;
  CODE128_START_C = 105;
  CODE128_MODULO = 103;
  CODE128_STOP = 106;
  CODE128_SHIFT = 98;
  CODE128_FNC1_CHAR = 207;
  CODE128_CODE_C_CHAR = 204;
  CODE128_CODE_B_CHAR = 205;
  CODE128_CODE_A_CHAR = 206;
  CODE128_START_A_CHAR = 208;
  CODE128_START_B_CHAR = 209;
  CODE128_START_C_CHAR = 210;

  CODE128_BARS: array[0..106] of string = (
    '11011001100', '11001101100', '11001100110', '10010011000',
    '10010001100', '10001001100', '10011001000', '10011000100',
    '10001100100', '11001001000', '11001000100', '11000100100',
    '10110011100', '10011011100', '10011001110', '10111001100',
    '10011101100', '10011100110', '11001110010', '11001011100',
    '11001001110', '11011100100', '11001110100', '11101101110',
    '11101001100', '11100101100', '11100100110', '11101100100',
    '11100110100', '11100110010', '11011011000', '11011000110',
    '11000110110', '10100011000', '10001011000', '10001000110',
    '10110001000', '10001101000', '10001100010', '11010001000',
    '11000101000', '11000100010', '10110111000', '10110001110',
    '10001101110', '10111011000', '10111000110', '10001110110',
    '11101110110', '11010001110', '11000101110', '11011101000',
    '11011100010', '11011101110', '11101011000', '11101000110',
    '11100010110', '11101101000', '11101100010', '11100011010',
    '11101111010', '11001000010', '11110001010', '10100110000',
    '10100001100', '10010110000', '10010000110', '10000101100',
    '10000100110', '10110010000', '10110000100', '10011010000',
    '10011000010', '10000110100', '10000110010', '11000010010',
    '11001010000', '11110111010', '11000010100', '10001111010',
    '10100111100', '10010111100', '10010011110', '10111100100',
    '10011110100', '10011110010', '11110100100', '11110010100',
    '11110010010', '11011011110', '11011110110', '11110110110',
    '10101111000', '10100011110', '10001011110', '10111101000',
    '10111100010', '11110101000', '11110100010', '10111011110',
    '10111101110', '11101011110', '11110101110', '11010000100',
    '11010010000', '11010011100', '1100011101011'
  );

  CODE39_CHARS = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%*';
  CODE39_ENCODINGS: array[0..43] of Integer = (
    20957, 29783, 23639, 30485, 20951, 29813, 23669, 20855,
    29789, 23645, 29975, 23831, 30533, 22295, 30149, 24005,
    21623, 29981, 23837, 22301, 30023, 23879, 30545, 22343,
    30161, 24017, 21959, 30065, 23921, 22385, 29015, 18263,
    29141, 17879, 29045, 18293, 17783, 29021, 18269, 17477,
    17489, 17681, 20753, 35770
  );

  EAN_SIDE_BIN = '101';
  EAN_MIDDLE_BIN = '01010';
  EAN_L: array[0..9] of string = (
    '0001101', '0011001', '0010011', '0111101', '0100011',
    '0110001', '0101111', '0111011', '0110111', '0001011'
  );
  EAN_G: array[0..9] of string = (
    '0100111', '0110011', '0011011', '0100001', '0011101',
    '0111001', '0000101', '0010001', '0001001', '0010111'
  );
  EAN_R: array[0..9] of string = (
    '1110010', '1100110', '1101100', '1000010', '1011100',
    '1001110', '1010000', '1000100', '1001000', '1110100'
  );
  EAN2_STRUCTURE: array[0..3] of string = ('LL', 'LG', 'GL', 'GG');
  EAN5_STRUCTURE: array[0..9] of string = (
    'GGLLL', 'GLGLL', 'GLLGL', 'GLLLG', 'LGGLL',
    'LLGGL', 'LLLGG', 'LGLGL', 'LGLLG', 'LLGLG'
  );
  EAN13_STRUCTURE: array[0..9] of string = (
    'LLLLLL', 'LLGLGG', 'LLGGLG', 'LLGGGL', 'LGLLGG',
    'LGGLLG', 'LGGGLL', 'LGLGLG', 'LGLGGL', 'LGGLGL'
  );

  UPCE_EXPANSIONS: array[0..9] of string = (
    'XX00000XXX', 'XX10000XXX', 'XX20000XXX', 'XXX00000XX',
    'XXXX00000X', 'XXXXX00005', 'XXXXX00006', 'XXXXX00007',
    'XXXXX00008', 'XXXXX00009'
  );
  UPCE_PARITY_0: array[0..9] of string = (
    'EEEOOO', 'EEOEOO', 'EEOOEO', 'EEOOOE', 'EOEEOO',
    'EOOEEO', 'EOOOEE', 'EOEOEO', 'EOEOOE', 'EOOEOE'
  );
  UPCE_PARITY_1: array[0..9] of string = (
    'OOOEEE', 'OOEOEE', 'OOEEOE', 'OOEEEO', 'OEOOEE',
    'OEEOOE', 'OEEEOO', 'OEOEOE', 'OEOEEO', 'OEEOEO'
  );

  ITF_START_BIN = '1010';
  ITF_END_BIN = '11101';
  ITF_BINARIES: array[0..9] of string = (
    '00110', '10001', '01001', '11000', '00101',
    '10100', '01100', '00011', '10010', '01010'
  );

  CODABAR_CHARS = '0123456789-$:/.+ABCD';
  CODABAR_ENCODINGS: array[0..19] of string = (
    '101010011', '101011001', '101001011', '110010101',
    '101101001', '110101001', '100101011', '100101101',
    '100110101', '110100101', '101001101', '101100101',
    '1101011011', '1101101011', '1101101101', '1011011011',
    '1011001001', '1001001011', '1010010011', '1010011001'
  );

  CODE93_SYMBOLS: array[0..47] of string = (
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.', ' ', '$',
    '/', '+', '%', '($)', '(%)', '(/)', '(+)', #255
  );
  CODE93_BINARIES: array[0..47] of string = (
    '100010100', '101001000', '101000100', '101000010',
    '100101000', '100100100', '100100010', '101010000',
    '100010010', '100001010', '110101000', '110100100',
    '110100010', '110010100', '110010010', '110001010',
    '101101000', '101100100', '101100010', '100110100',
    '100011010', '101011000', '101001100', '101000110',
    '100101100', '100010110', '110110100', '110110010',
    '110101100', '110100110', '110010110', '110011010',
    '101101100', '101100110', '100110110', '100111010',
    '100101110', '111010100', '111010010', '111001010',
    '101101110', '101110110', '110101110', '100100110',
    '111011010', '111010110', '100110010', '101011110'
  );

function Fail(const Msg: string; var Error: string): Boolean;
begin
  Error := Msg;
  Result := False;
end;

function CharToDigit(C: Char): Integer;
begin
  Result := Ord(C) - Ord('0');
end;

function IsDigit(C: Char): Boolean;
begin
  Result := (C >= '0') and (C <= '9');
end;

function IsDigits(const S: string; Count: Integer): Boolean;
var
  I: Integer;
begin
  Result := (Count < 0) or (Length(S) = Count);
  if not Result then
    Exit;

  Result := Length(S) > 0;
  for I := 1 to Length(S) do
  begin
    if not IsDigit(S[I]) then
    begin
      Result := False;
      Exit;
    end;
  end;
end;

function IntToBinary(Value: Integer): string;
begin
  if Value = 0 then
  begin
    Result := '0';
    Exit;
  end;

  Result := '';
  while Value > 0 do
  begin
    if (Value and 1) = 0 then
      Result := '0' + Result
    else
      Result := '1' + Result;
    Value := Value shr 1;
  end;
end;

function PrintableAsciiOnly(const S: string): string;
var
  I, N: Integer;
begin
  Result := '';
  for I := 1 to Length(S) do
  begin
    N := Ord(S[I]);
    if (N >= 32) and (N <= 126) then
      Result := Result + S[I];
  end;
end;

function Code39CharValue(C: Char): Integer;
begin
  Result := Pos(C, CODE39_CHARS) - 1;
end;

function Code39Encoding(C: Char): string;
var
  Index: Integer;
begin
  Index := Code39CharValue(C);
  if (Index < Low(CODE39_ENCODINGS)) or (Index > High(CODE39_ENCODINGS)) then
    Result := ''
  else
    Result := IntToBinary(CODE39_ENCODINGS[Index]);
end;

function EncodeCode39(const Value: string; UseMod43: Boolean; out Pattern,
  Text, Error: string): Boolean;
var
  Data: string;
  I, Sum, CheckValue: Integer;
begin
  Data := AnsiUpperCase(Value);
  if Data = '' then
    Exit(Fail('CODE39 requires at least one character.', Error));

  for I := 1 to Length(Data) do
  begin
    if Code39CharValue(Data[I]) < 0 then
      Exit(Fail('CODE39 accepts digits, uppercase letters, space, and - . $ / + %.', Error));
  end;

  if UseMod43 then
  begin
    Sum := 0;
    for I := 1 to Length(Data) do
      Inc(Sum, Code39CharValue(Data[I]));
    CheckValue := Sum mod 43;
    Data := Data + CODE39_CHARS[CheckValue + 1];
  end;

  Pattern := Code39Encoding('*');
  for I := 1 to Length(Data) do
    Pattern := Pattern + Code39Encoding(Data[I]) + '0';
  Pattern := Pattern + Code39Encoding('*');
  Text := Data;
  Result := True;
end;

function IsCode128Allowed(C: Char): Boolean;
var
  N: Integer;
begin
  N := Ord(C);
  Result := ((N >= 0) and (N <= 127)) or ((N >= 200) and (N <= 211));
end;

function IsCode128SetAChar(C: Char): Boolean;
var
  N: Integer;
begin
  N := Ord(C);
  Result := ((N >= 0) and (N <= 95)) or ((N >= 200) and (N <= 207));
end;

function IsCode128SetBChar(C: Char): Boolean;
var
  N: Integer;
begin
  N := Ord(C);
  Result := ((N >= 32) and (N <= 127)) or ((N >= 200) and (N <= 207));
end;

function CountSetALength(const S: string; PosIndex: Integer): Integer;
begin
  Result := 0;
  while (PosIndex + Result <= Length(S)) and
    IsCode128SetAChar(S[PosIndex + Result]) do
    Inc(Result);
end;

function CountSetBLength(const S: string; PosIndex: Integer): Integer;
begin
  Result := 0;
  while (PosIndex + Result <= Length(S)) and
    IsCode128SetBChar(S[PosIndex + Result]) do
    Inc(Result);
end;

function CountDigitRun(const S: string; PosIndex: Integer): Integer;
begin
  Result := 0;
  while (PosIndex + Result <= Length(S)) and IsDigit(S[PosIndex + Result]) do
    Inc(Result);
end;

function CountSetCLength(const S: string; PosIndex: Integer): Integer;
var
  P, StartP: Integer;
begin
  P := PosIndex;
  while P <= Length(S) do
  begin
    StartP := P;
    while (P <= Length(S)) and (Ord(S[P]) = CODE128_FNC1_CHAR) do
      Inc(P);

    if (P < Length(S)) and IsDigit(S[P]) and IsDigit(S[P + 1]) then
    begin
      Inc(P, 2);
      while (P <= Length(S)) and (Ord(S[P]) = CODE128_FNC1_CHAR) do
        Inc(P);
    end
    else
    begin
      P := StartP;
      Break;
    end;
  end;
  Result := P - PosIndex;
end;

function ChooseCode128ABSet(const S: string; PosIndex: Integer): Integer;
begin
  if CountSetALength(S, PosIndex) > CountSetBLength(S, PosIndex) then
    Result := CODE128_SET_A
  else
    Result := CODE128_SET_B;
end;

function Code128StartChar(ASet: Integer): Char;
begin
  case ASet of
    CODE128_SET_A:
      Result := Chr(CODE128_START_A_CHAR);
    CODE128_SET_C:
      Result := Chr(CODE128_START_C_CHAR);
  else
    Result := Chr(CODE128_START_B_CHAR);
  end;
end;

function Code128SwitchChar(ASet: Integer): Char;
begin
  case ASet of
    CODE128_SET_A:
      Result := Chr(CODE128_CODE_A_CHAR);
    CODE128_SET_C:
      Result := Chr(CODE128_CODE_C_CHAR);
  else
    Result := Chr(CODE128_CODE_B_CHAR);
  end;
end;

function Code128AutoString(const S: string): string;
var
  CurrentSet, DigitRun, P, StartCLength, TargetSet: Integer;
begin
  StartCLength := CountSetCLength(S, 1);
  if StartCLength >= 2 then
    CurrentSet := CODE128_SET_C
  else
    CurrentSet := ChooseCode128ABSet(S, 1);

  Result := Code128StartChar(CurrentSet);
  P := 1;

  while P <= Length(S) do
  begin
    if CurrentSet = CODE128_SET_C then
    begin
      if Ord(S[P]) = CODE128_FNC1_CHAR then
      begin
        Result := Result + S[P];
        Inc(P);
      end
      else if (P < Length(S)) and IsDigit(S[P]) and IsDigit(S[P + 1]) then
      begin
        Result := Result + S[P] + S[P + 1];
        Inc(P, 2);
      end
      else
      begin
        CurrentSet := ChooseCode128ABSet(S, P);
        Result := Result + Code128SwitchChar(CurrentSet);
      end;
    end
    else
    begin
      DigitRun := CountDigitRun(S, P);
      if DigitRun >= 4 then
      begin
        if Odd(DigitRun) then
        begin
          Result := Result + S[P];
          Inc(P);
        end
        else
        begin
          CurrentSet := CODE128_SET_C;
          Result := Result + Code128SwitchChar(CurrentSet);
        end;
      end
      else if ((CurrentSet = CODE128_SET_A) and IsCode128SetAChar(S[P])) or
        ((CurrentSet = CODE128_SET_B) and IsCode128SetBChar(S[P])) then
      begin
        Result := Result + S[P];
        Inc(P);
      end
      else if Ord(S[P]) >= 200 then
      begin
        Result := Result + S[P];
        Inc(P);
      end
      else
      begin
        if CurrentSet = CODE128_SET_A then
          TargetSet := CODE128_SET_B
        else
          TargetSet := CODE128_SET_A;
        CurrentSet := TargetSet;
        Result := Result + Code128SwitchChar(CurrentSet);
      end;
    end;
  end;
end;

function Code128SetByStartIndex(StartIndex: Integer): Integer;
begin
  case StartIndex of
    CODE128_START_A:
      Result := CODE128_SET_A;
    CODE128_START_B:
      Result := CODE128_SET_B;
    CODE128_START_C:
      Result := CODE128_SET_C;
  else
    Result := -1;
  end;
end;

function Code128SetBySwitchIndex(Index: Integer): Integer;
begin
  case Index of
    101:
      Result := CODE128_SET_A;
    100:
      Result := CODE128_SET_B;
    99:
      Result := CODE128_SET_C;
  else
    Result := -1;
  end;
end;

function Code128CorrectIndex(const Bytes: TIntArray; var P: Integer;
  CurrentSet: Integer; var Error: string): Integer;
var
  C1, C2: Integer;
begin
  Result := -1;
  if P > Length(Bytes) then
  begin
    Error := 'Unexpected end of CODE128 data.';
    Exit;
  end;

  case CurrentSet of
    CODE128_SET_A:
      begin
        C1 := Bytes[P - 1];
        Inc(P);
        if C1 < 32 then
          Result := C1 + 64
        else
          Result := C1 - 32;
      end;
    CODE128_SET_B:
      begin
        C1 := Bytes[P - 1];
        Inc(P);
        Result := C1 - 32;
      end;
    CODE128_SET_C:
      begin
        if P >= Length(Bytes) then
        begin
          Error := 'CODE128C requires an even number of digits.';
          Exit;
        end;
        C1 := Bytes[P - 1];
        C2 := Bytes[P];
        if (C1 < Ord('0')) or (C1 > Ord('9')) or
          (C2 < Ord('0')) or (C2 > Ord('9')) then
        begin
          Error := 'CODE128C can only encode digit pairs and FNC1.';
          Exit;
        end;
        Inc(P, 2);
        Result := (C1 - Ord('0')) * 10 + C2 - Ord('0');
      end;
  end;

  if (Result < 0) or (Result > 102) then
    Error := 'Character is not valid for the selected CODE128 code set.';
end;

function EncodeCode128Prepared(const Prepared: string; EAN128: Boolean;
  out Pattern, Text, Error: string): Boolean;
var
  Bytes: TIntArray;
  Checksum, CurrentSet, I, Index, NextSet, P, PosWeight, StartIndex: Integer;
  Encoded: string;
begin
  Result := False;
  if Prepared = '' then
    Exit(Fail('CODE128 requires at least one character.', Error));

  StartIndex := Ord(Prepared[1]) - 105;
  CurrentSet := Code128SetByStartIndex(StartIndex);
  if CurrentSet < 0 then
    Exit(Fail('The CODE128 encoding does not start with a start character.', Error));

  SetLength(Bytes, Length(Prepared) - 1);
  for I := 2 to Length(Prepared) do
    Bytes[I - 2] := Ord(Prepared[I]);

  if EAN128 then
  begin
    SetLength(Bytes, Length(Bytes) + 1);
    for I := High(Bytes) downto 1 do
      Bytes[I] := Bytes[I - 1];
    Bytes[0] := CODE128_FNC1_CHAR;
  end;

  Encoded := '';
  Checksum := 0;
  PosWeight := 1;
  P := 1;
  while P <= Length(Bytes) do
  begin
    if Bytes[P - 1] >= 200 then
    begin
      Index := Bytes[P - 1] - 105;
      Inc(P);
      NextSet := Code128SetBySwitchIndex(Index);
      if NextSet >= 0 then
        CurrentSet := NextSet
      else if ((CurrentSet = CODE128_SET_A) or (CurrentSet = CODE128_SET_B)) and
        (Index = CODE128_SHIFT) and (P <= Length(Bytes)) then
      begin
        if CurrentSet = CODE128_SET_A then
        begin
          if Bytes[P - 1] > 95 then
            Dec(Bytes[P - 1], 96);
        end
        else
        begin
          if Bytes[P - 1] < 32 then
            Inc(Bytes[P - 1], 96);
        end;
      end;
    end
    else
    begin
      Index := Code128CorrectIndex(Bytes, P, CurrentSet, Error);
      if Error <> '' then
        Exit;
    end;

    if (Index < 0) or (Index > CODE128_STOP) then
      Exit(Fail('CODE128 produced an invalid symbol index.', Error));

    Encoded := Encoded + CODE128_BARS[Index];
    Inc(Checksum, Index * PosWeight);
    Inc(PosWeight);
  end;

  Pattern := CODE128_BARS[StartIndex] + Encoded +
    CODE128_BARS[(Checksum + StartIndex) mod CODE128_MODULO] +
    CODE128_BARS[CODE128_STOP];
  Text := PrintableAsciiOnly(Copy(Prepared, 2, MaxInt));
  Result := True;
end;

function ValidateCode128Prepared(const Data: string; out Error: string): Boolean;
var
  I: Integer;
begin
  if Data = '' then
    Exit(Fail('CODE128 requires at least one character.', Error));

  for I := 1 to Length(Data) do
  begin
    if not IsCode128Allowed(Data[I]) then
      Exit(Fail('CODE128 accepts ASCII characters and JsBarcode special codes #200..#211.', Error));
  end;
  Result := True;
end;

function ValidateCode128Set(const Data: string; ASet: Integer; out Error: string): Boolean;
var
  I: Integer;
begin
  if Data = '' then
    Exit(Fail('CODE128 requires at least one character.', Error));

  if ASet = CODE128_SET_C then
  begin
    if CountSetCLength(Data, 1) <> Length(Data) then
      Exit(Fail('CODE128C requires digit pairs and optional FNC1 (#207).', Error));
  end
  else
  begin
    for I := 1 to Length(Data) do
    begin
      if (ASet = CODE128_SET_A) and not IsCode128SetAChar(Data[I]) then
        Exit(Fail('CODE128A accepts ASCII #0..#95 and JsBarcode special codes #200..#207.', Error));
      if (ASet = CODE128_SET_B) and not IsCode128SetBChar(Data[I]) then
        Exit(Fail('CODE128B accepts ASCII #32..#127 and JsBarcode special codes #200..#207.', Error));
    end;
  end;
  Result := True;
end;

function EncodeCode128(const Value: string; AType: TTMSBarcodeType; EAN128: Boolean;
  out Pattern, Text, Error: string): Boolean;
var
  Prepared: string;
begin
  case AType of
    tbcCODE128A:
      begin
        if not ValidateCode128Set(Value, CODE128_SET_A, Error) then
          Exit(False);
        Prepared := Chr(CODE128_START_A_CHAR) + Value;
      end;
    tbcCODE128B:
      begin
        if not ValidateCode128Set(Value, CODE128_SET_B, Error) then
          Exit(False);
        Prepared := Chr(CODE128_START_B_CHAR) + Value;
      end;
    tbcCODE128C:
      begin
        if not ValidateCode128Set(Value, CODE128_SET_C, Error) then
          Exit(False);
        Prepared := Chr(CODE128_START_C_CHAR) + Value;
      end;
  else
    if not ValidateCode128Prepared(Value, Error) then
      Exit(False);
    Prepared := Code128AutoString(Value);
  end;

  Result := EncodeCode128Prepared(Prepared, EAN128, Pattern, Text, Error);
end;

function EANCode(const Digit: Char; Structure: Char): string;
var
  N: Integer;
begin
  N := CharToDigit(Digit);
  case Structure of
    'G', 'E':
      Result := EAN_G[N];
    'R':
      Result := EAN_R[N];
  else
    Result := EAN_L[N];
  end;
end;

function EANEncodeDigits(const Data, Structure, Separator: string): string;
var
  I: Integer;
begin
  Result := '';
  for I := 1 to Length(Data) do
  begin
    Result := Result + EANCode(Data[I], Structure[I]);
    if (Separator <> '') and (I < Length(Data)) then
      Result := Result + Separator;
  end;
end;

function EAN13Checksum(const Number: string): Integer;
var
  I, Sum: Integer;
begin
  Sum := 0;
  for I := 1 to 12 do
  begin
    if Odd(I) then
      Inc(Sum, CharToDigit(Number[I]))
    else
      Inc(Sum, CharToDigit(Number[I]) * 3);
  end;
  Result := (10 - (Sum mod 10)) mod 10;
end;

function EAN8Checksum(const Number: string): Integer;
var
  I, Sum: Integer;
begin
  Sum := 0;
  for I := 1 to 7 do
  begin
    if Odd(I) then
      Inc(Sum, CharToDigit(Number[I]) * 3)
    else
      Inc(Sum, CharToDigit(Number[I]));
  end;
  Result := (10 - (Sum mod 10)) mod 10;
end;

function EAN5Checksum(const Data: string): Integer;
var
  I, Sum: Integer;
begin
  Sum := 0;
  for I := 1 to 5 do
  begin
    if Odd(I) then
      Inc(Sum, CharToDigit(Data[I]) * 3)
    else
      Inc(Sum, CharToDigit(Data[I]) * 9);
  end;
  Result := Sum mod 10;
end;

function UPCChecksum(const Number: string): Integer;
var
  I, Sum: Integer;
begin
  Sum := 0;
  for I := 2 to 11 do
  begin
    if not Odd(I) then
      Inc(Sum, CharToDigit(Number[I]));
  end;
  for I := 1 to 11 do
  begin
    if Odd(I) then
      Inc(Sum, CharToDigit(Number[I]) * 3);
  end;
  Result := (10 - (Sum mod 10)) mod 10;
end;

function EncodeEAN13(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  Data: string;
begin
  Data := Value;
  if IsDigits(Data, 12) then
    Data := Data + Chr(Ord('0') + EAN13Checksum(Data));

  if not IsDigits(Data, 13) then
    Exit(Fail('EAN13 requires 12 digits or 13 digits with a valid checksum.', Error));
  if CharToDigit(Data[13]) <> EAN13Checksum(Data) then
    Exit(Fail('EAN13 requires 12 digits or 13 digits with a valid checksum.', Error));

  Pattern := EAN_SIDE_BIN +
    EANEncodeDigits(Copy(Data, 2, 6), EAN13_STRUCTURE[CharToDigit(Data[1])], '') +
    EAN_MIDDLE_BIN +
    EANEncodeDigits(Copy(Data, 8, 6), 'RRRRRR', '') +
    EAN_SIDE_BIN;
  Text := Data;
  Result := True;
end;

function EncodeEAN8(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  Data: string;
begin
  Data := Value;
  if IsDigits(Data, 7) then
    Data := Data + Chr(Ord('0') + EAN8Checksum(Data));

  if not IsDigits(Data, 8) then
    Exit(Fail('EAN8 requires 7 digits or 8 digits with a valid checksum.', Error));
  if CharToDigit(Data[8]) <> EAN8Checksum(Data) then
    Exit(Fail('EAN8 requires 7 digits or 8 digits with a valid checksum.', Error));

  Pattern := EAN_SIDE_BIN +
    EANEncodeDigits(Copy(Data, 1, 4), 'LLLL', '') +
    EAN_MIDDLE_BIN +
    EANEncodeDigits(Copy(Data, 5, 4), 'RRRR', '') +
    EAN_SIDE_BIN;
  Text := Data;
  Result := True;
end;

function EncodeEAN5(const Value: string; out Pattern, Text, Error: string): Boolean;
begin
  if not IsDigits(Value, 5) then
    Exit(Fail('EAN5 requires exactly 5 digits.', Error));

  Pattern := '1011' + EANEncodeDigits(Value, EAN5_STRUCTURE[EAN5Checksum(Value)], '01');
  Text := Value;
  Result := True;
end;

function EncodeEAN2(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  N: Integer;
begin
  if not IsDigits(Value, 2) then
    Exit(Fail('EAN2 requires exactly 2 digits.', Error));

  N := StrToInt(Value);
  Pattern := '1011' + EANEncodeDigits(Value, EAN2_STRUCTURE[N mod 4], '01');
  Text := Value;
  Result := True;
end;

function EncodeUPC(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  Data: string;
begin
  Data := Value;
  if IsDigits(Data, 11) then
    Data := Data + Chr(Ord('0') + UPCChecksum(Data));

  if not IsDigits(Data, 12) then
    Exit(Fail('UPC requires 11 digits or 12 digits with a valid checksum.', Error));
  if CharToDigit(Data[12]) <> UPCChecksum(Data) then
    Exit(Fail('UPC requires 11 digits or 12 digits with a valid checksum.', Error));

  Pattern := '101' +
    EANEncodeDigits(Copy(Data, 1, 6), 'LLLLLL', '') +
    '01010' +
    EANEncodeDigits(Copy(Data, 7, 6), 'RRRRRR', '') +
    '101';
  Text := Data;
  Result := True;
end;

function ExpandUPCEToUPCA(const MiddleDigits, NumberSystem: string): string;
var
  C: Char;
  DigitIndex, I, LastDigit: Integer;
  Expanded: string;
begin
  LastDigit := CharToDigit(MiddleDigits[Length(MiddleDigits)]);
  Expanded := '';
  DigitIndex := 1;
  for I := 1 to Length(UPCE_EXPANSIONS[LastDigit]) do
  begin
    C := UPCE_EXPANSIONS[LastDigit][I];
    if C = 'X' then
    begin
      Expanded := Expanded + MiddleDigits[DigitIndex];
      Inc(DigitIndex);
    end
    else
      Expanded := Expanded + C;
  end;

  Result := NumberSystem + Expanded;
  Result := Result + Chr(Ord('0') + UPCChecksum(Result));
end;

function EncodeUPCE(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  CheckDigit, NumberSystem: Integer;
  MiddleDigits, Parity, UpcA: string;
begin
  if IsDigits(Value, 6) then
  begin
    MiddleDigits := Value;
    UpcA := ExpandUPCEToUPCA(MiddleDigits, '0');
    Text := UpcA[1] + MiddleDigits + UpcA[Length(UpcA)];
  end
  else if IsDigits(Value, 8) and ((Value[1] = '0') or (Value[1] = '1')) then
  begin
    MiddleDigits := Copy(Value, 2, 6);
    UpcA := ExpandUPCEToUPCA(MiddleDigits, Value[1]);
    if UpcA[Length(UpcA)] <> Value[8] then
      Exit(Fail('UPCE checksum does not match the expanded UPC-A value.', Error));
    Text := Value;
  end
  else
    Exit(Fail('UPCE requires 6 digits or 8 digits beginning with 0 or 1.', Error));

  NumberSystem := CharToDigit(UpcA[1]);
  CheckDigit := CharToDigit(UpcA[Length(UpcA)]);
  if NumberSystem = 0 then
    Parity := UPCE_PARITY_0[CheckDigit]
  else
    Parity := UPCE_PARITY_1[CheckDigit];

  Pattern := '101' + EANEncodeDigits(MiddleDigits, Parity, '') + '010101';
  Result := True;
end;

function ITF14Checksum(const Data: string): Integer;
var
  I, Sum, Weight: Integer;
begin
  Sum := 0;
  for I := 1 to 13 do
  begin
    if Odd(I) then
      Weight := 3
    else
      Weight := 1;
    Inc(Sum, CharToDigit(Data[I]) * Weight);
  end;
  Result := (10 - (Sum mod 10)) mod 10;
end;

function EncodeITFPair(const Pair: string): string;
var
  First, Second: string;
  I: Integer;
begin
  First := ITF_BINARIES[CharToDigit(Pair[1])];
  Second := ITF_BINARIES[CharToDigit(Pair[2])];
  Result := '';
  for I := 1 to 5 do
  begin
    if First[I] = '1' then
      Result := Result + '111'
    else
      Result := Result + '1';

    if Second[I] = '1' then
      Result := Result + '000'
    else
      Result := Result + '0';
  end;
end;

function EncodeITF(const Value: string; ITF14: Boolean; out Pattern, Text,
  Error: string): Boolean;
var
  Data: string;
  I: Integer;
begin
  Data := Value;
  if ITF14 then
  begin
    if IsDigits(Data, 13) then
      Data := Data + Chr(Ord('0') + ITF14Checksum(Data));
    if not IsDigits(Data, 14) then
      Exit(Fail('ITF14 requires 13 digits or 14 digits with a valid checksum.', Error));
    if CharToDigit(Data[14]) <> ITF14Checksum(Data) then
      Exit(Fail('ITF14 requires 13 digits or 14 digits with a valid checksum.', Error));
  end
  else
  begin
    if (not IsDigits(Data, -1)) or Odd(Length(Data)) then
      Exit(Fail('ITF requires an even number of digits.', Error));
  end;

  Pattern := ITF_START_BIN;
  I := 1;
  while I < Length(Data) do
  begin
    Pattern := Pattern + EncodeITFPair(Copy(Data, I, 2));
    Inc(I, 2);
  end;
  Pattern := Pattern + ITF_END_BIN;
  Text := Data;
  Result := True;
end;

function MSIMod10(const Number: string): Integer;
var
  I, N, Sum, Twice: Integer;
begin
  Sum := 0;
  for I := 1 to Length(Number) do
  begin
    N := CharToDigit(Number[I]);
    if ((I - 1 + Length(Number)) mod 2) = 0 then
      Inc(Sum, N)
    else
    begin
      Twice := N * 2;
      Inc(Sum, (Twice mod 10) + (Twice div 10));
    end;
  end;
  Result := (10 - (Sum mod 10)) mod 10;
end;

function MSIMod11(const Number: string): Integer;
const
  Weights: array[0..5] of Integer = (2, 3, 4, 5, 6, 7);
var
  I, N, Sum: Integer;
begin
  Sum := 0;
  for I := 0 to Length(Number) - 1 do
  begin
    N := CharToDigit(Number[Length(Number) - I]);
    Inc(Sum, Weights[I mod Length(Weights)] * N);
  end;
  Result := (11 - (Sum mod 11)) mod 11;
end;

function EncodeMSI(const Value: string; AType: TTMSBarcodeType; out Pattern,
  Text, Error: string): Boolean;
var
  Bin, Data: string;
  B, Digit, I: Integer;
begin
  if not IsDigits(Value, -1) then
    Exit(Fail('MSI requires one or more digits.', Error));

  Data := Value;
  case AType of
    tbcMSI10:
      Data := Data + IntToStr(MSIMod10(Data));
    tbcMSI11:
      Data := Data + IntToStr(MSIMod11(Data));
    tbcMSI1010:
      begin
        Data := Data + IntToStr(MSIMod10(Data));
        Data := Data + IntToStr(MSIMod10(Data));
      end;
    tbcMSI1110:
      begin
        Data := Data + IntToStr(MSIMod11(Data));
        Data := Data + IntToStr(MSIMod10(Data));
      end;
  end;

  Pattern := '110';
  for I := 1 to Length(Data) do
  begin
    Digit := CharToDigit(Data[I]);
    Bin := '';
    for B := 3 downto 0 do
    begin
      if (Digit and (1 shl B)) = 0 then
        Bin := Bin + '0'
      else
        Bin := Bin + '1';
    end;

    for B := 1 to Length(Bin) do
    begin
      if Bin[B] = '0' then
        Pattern := Pattern + '100'
      else
        Pattern := Pattern + '110';
    end;
  end;
  Pattern := Pattern + '1001';
  Text := Data;
  Result := True;
end;

function EncodePharmacode(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  Code: Integer;
begin
  if (Value = '') or (not TryStrToInt(Value, Code)) or
    (Code < 3) or (Code > 131070) then
    Exit(Fail('pharmacode requires a number from 3 through 131070.', Error));

  Pattern := '';
  while Code <> 0 do
  begin
    if (Code mod 2) = 0 then
    begin
      Pattern := '11100' + Pattern;
      Code := (Code - 2) div 2;
    end
    else
    begin
      Pattern := '100' + Pattern;
      Code := (Code - 1) div 2;
    end;
  end;

  Delete(Pattern, Length(Pattern) - 1, 2);
  Text := Value;
  Result := True;
end;

function CodabarEncoding(C: Char): string;
var
  Index: Integer;
begin
  Index := Pos(C, CODABAR_CHARS) - 1;
  if Index < 0 then
    Result := ''
  else
    Result := CODABAR_ENCODINGS[Index];
end;

function EncodeCodabar(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  Data: string;
  I: Integer;
  NeedsGuards: Boolean;
begin
  Data := AnsiUpperCase(Value);
  if Data = '' then
    Exit(Fail('codabar requires data.', Error));

  NeedsGuards := True;
  for I := 1 to Length(Data) do
  begin
    if Pos(Data[I], '0123456789-$:/.+') = 0 then
    begin
      NeedsGuards := False;
      Break;
    end;
  end;
  if NeedsGuards then
    Data := 'A' + Data + 'A';

  if (Length(Data) < 3) or (Pos(Data[1], 'ABCD') = 0) or
    (Pos(Data[Length(Data)], 'ABCD') = 0) then
    Exit(Fail('codabar requires A-D start/end guards, or plain digits and - $ : . + /.', Error));

  for I := 2 to Length(Data) - 1 do
  begin
    if Pos(Data[I], '0123456789-$:/.+') = 0 then
      Exit(Fail('codabar body accepts digits and - $ : . + /.', Error));
  end;

  Pattern := '';
  for I := 1 to Length(Data) do
  begin
    Pattern := Pattern + CodabarEncoding(Data[I]);
    if I <> Length(Data) then
      Pattern := Pattern + '0';
  end;

  Text := StringReplace(Data, 'A', '', [rfReplaceAll]);
  Text := StringReplace(Text, 'B', '', [rfReplaceAll]);
  Text := StringReplace(Text, 'C', '', [rfReplaceAll]);
  Text := StringReplace(Text, 'D', '', [rfReplaceAll]);
  Result := True;
end;

procedure AddSymbol(var Symbols: TStringArray; const Symbol: string);
var
  L: Integer;
begin
  L := Length(Symbols);
  SetLength(Symbols, L + 1);
  Symbols[L] := Symbol;
end;

function CopySymbols(const Symbols: TStringArray): TStringArray;
var
  I: Integer;
begin
  SetLength(Result, Length(Symbols));
  for I := 0 to High(Symbols) do
    Result[I] := Symbols[I];
end;

function Code93SymbolValue(const Symbol: string): Integer;
var
  I: Integer;
begin
  for I := Low(CODE93_SYMBOLS) to High(CODE93_SYMBOLS) do
  begin
    if CODE93_SYMBOLS[I] = Symbol then
    begin
      Result := I;
      Exit;
    end;
  end;
  Result := -1;
end;

function Code93Encoding(const Symbol: string): string;
var
  Index: Integer;
begin
  Index := Code93SymbolValue(Symbol);
  if Index < 0 then
    Result := ''
  else
    Result := CODE93_BINARIES[Index];
end;

function Code93Checksum(const Symbols: TStringArray; MaxWeight: Integer): string;
var
  I, Sum, Value, Weight: Integer;
begin
  Sum := 0;
  for I := High(Symbols) downto 0 do
  begin
    Weight := ((High(Symbols) - I) mod MaxWeight) + 1;
    Value := Code93SymbolValue(Symbols[I]);
    Inc(Sum, Value * Weight);
  end;
  Result := CODE93_SYMBOLS[Sum mod 47];
end;

function Code93AddFullAsciiChar(C: Char; var Symbols: TStringArray): Boolean;
var
  N: Integer;
begin
  Result := True;
  N := Ord(C);
  case N of
    0:
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, 'U'); end;
    1..26:
      begin AddSymbol(Symbols, '($)'); AddSymbol(Symbols, Chr(Ord('A') + N - 1)); end;
    27..31:
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, Chr(Ord('A') + N - 27)); end;
    Ord('!')..Ord('#'):
      begin AddSymbol(Symbols, '(/)'); AddSymbol(Symbols, Chr(Ord('A') + N - Ord('!'))); end;
    Ord('&')..Ord('*'):
      begin AddSymbol(Symbols, '(/)'); AddSymbol(Symbols, Chr(Ord('F') + N - Ord('&'))); end;
    Ord(','):
      begin AddSymbol(Symbols, '(/)'); AddSymbol(Symbols, 'L'); end;
    Ord(':'):
      begin AddSymbol(Symbols, '(/)'); AddSymbol(Symbols, 'Z'); end;
    Ord(';')..Ord('?'):
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, Chr(Ord('F') + N - Ord(';'))); end;
    Ord('@'):
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, 'V'); end;
    Ord('[')..Ord('_'):
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, Chr(Ord('K') + N - Ord('['))); end;
    Ord('`'):
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, 'W'); end;
    Ord('a')..Ord('z'):
      begin AddSymbol(Symbols, '(+)'); AddSymbol(Symbols, Chr(Ord('A') + N - Ord('a'))); end;
    Ord('{')..Ord('~'):
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, Chr(Ord('P') + N - Ord('{'))); end;
    127:
      begin AddSymbol(Symbols, '(%)'); AddSymbol(Symbols, 'T'); end;
  else
    if Code93SymbolValue(C) >= 0 then
      AddSymbol(Symbols, C)
    else
      Result := False;
  end;
end;

function EncodeCode93(const Value: string; FullASCII: Boolean; out Pattern,
  Text, Error: string): Boolean;
var
  CsumC, CsumK, SymbolText: string;
  I: Integer;
  Symbols, TempSymbols: TStringArray;
begin
  if Value = '' then
    Exit(Fail('CODE93 requires at least one character.', Error));

  SetLength(Symbols, 0);
  for I := 1 to Length(Value) do
  begin
    if FullASCII then
    begin
      if (Ord(Value[I]) > 127) or
        (not Code93AddFullAsciiChar(Value[I], Symbols)) then
        Exit(Fail('CODE93FullASCII accepts ASCII #0..#127.', Error));
    end
    else
    begin
      SymbolText := Value[I];
      if Code93SymbolValue(SymbolText) < 0 then
        Exit(Fail('CODE93 accepts digits, uppercase letters, space, and - . $ / + %.', Error));
      AddSymbol(Symbols, SymbolText);
    end;
  end;

  Pattern := Code93Encoding(#255);
  for I := 0 to High(Symbols) do
    Pattern := Pattern + Code93Encoding(Symbols[I]);

  CsumC := Code93Checksum(Symbols, 20);
  TempSymbols := CopySymbols(Symbols);
  AddSymbol(TempSymbols, CsumC);
  CsumK := Code93Checksum(TempSymbols, 15);

  Pattern := Pattern + Code93Encoding(CsumC) + Code93Encoding(CsumK) +
    Code93Encoding(#255) + '1';
  Text := Value;
  Result := True;
end;

function EncodeGenericBarcode(const Value: string; out Pattern, Text,
  Error: string): Boolean;
begin
  Pattern := '10101010101010101010101010101010101010101';
  Text := Value;
  Error := '';
  Result := True;
end;

function EncodeBarcodeValue(AType: TTMSBarcodeType; const Value: string;
  UseMod43, EAN128: Boolean; out Pattern, Text, Error: string): Boolean;
begin
  Pattern := '';
  Text := Value;
  Error := '';
  case AType of
    tbcCODE39:
      Result := EncodeCode39(Value, UseMod43, Pattern, Text, Error);
    tbcCODE128, tbcCODE128A, tbcCODE128B, tbcCODE128C:
      Result := EncodeCode128(Value, AType, EAN128, Pattern, Text, Error);
    tbcEAN13:
      Result := EncodeEAN13(Value, Pattern, Text, Error);
    tbcEAN8:
      Result := EncodeEAN8(Value, Pattern, Text, Error);
    tbcEAN5:
      Result := EncodeEAN5(Value, Pattern, Text, Error);
    tbcEAN2:
      Result := EncodeEAN2(Value, Pattern, Text, Error);
    tbcUPC:
      Result := EncodeUPC(Value, Pattern, Text, Error);
    tbcUPCE:
      Result := EncodeUPCE(Value, Pattern, Text, Error);
    tbcITF14:
      Result := EncodeITF(Value, True, Pattern, Text, Error);
    tbcITF:
      Result := EncodeITF(Value, False, Pattern, Text, Error);
    tbcMSI, tbcMSI10, tbcMSI11, tbcMSI1010, tbcMSI1110:
      Result := EncodeMSI(Value, AType, Pattern, Text, Error);
    tbcPharmacode:
      Result := EncodePharmacode(Value, Pattern, Text, Error);
    tbcCodabar:
      Result := EncodeCodabar(Value, Pattern, Text, Error);
    tbcCODE93:
      Result := EncodeCode93(Value, False, Pattern, Text, Error);
    tbcCODE93FullASCII:
      Result := EncodeCode93(Value, True, Pattern, Text, Error);
    tbcGenericBarcode:
      Result := EncodeGenericBarcode(Value, Pattern, Text, Error);
  else
    Result := Fail('Unsupported barcode type.', Error);
  end;
end;

function TMSBarcodeTypeName(AType: TTMSBarcodeType): string;
begin
  Result := BarcodeTypeNames[AType];
end;

function TMSBarcodeTypeFromName(const AName: string; out AType: TTMSBarcodeType): Boolean;
var
  T: TTMSBarcodeType;
begin
  for T := Low(TTMSBarcodeType) to High(TTMSBarcodeType) do
  begin
    if SameText(AName, BarcodeTypeNames[T]) then
    begin
      AType := T;
      Result := True;
      Exit;
    end;
  end;
  Result := False;
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

{ TTMSFNCBarCode }

constructor TTMSFNCBarCode.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  Width := 320;
  Height := 140;
  FBackgroundColor := gcWhite;
  FBarcodeType := tbcCODE128;
  FBarColor := gcBlack;
  FBarHeight := 100;
  FDisplayValue := True;
  FEAN128 := False;
  FMargin := 10;
  FMod43Checksum := False;
  FModuleWidth := 2;
  FTextMargin := 2;
  FValue := '123456789012';
  Color := FBackgroundColor;
  {$IFDEF FMXLIB}
  FTextFont := TTMSFNCGraphicsFont.Create;
  FTextFont.OnChanged := FontChanged;
  FTextFont.Name := 'Consolas';
  FTextFont.Size := 12;
  FTextFont.Color := gcBlack;
  {$ELSE}
  Font.Name := 'Consolas';
  Font.Size := 12;
  Font.Color := gcBlack;
  {$ENDIF}
end;

destructor TTMSFNCBarCode.Destroy;
begin
  {$IFDEF FMXLIB}
  FTextFont.Free;
  {$ENDIF}
  inherited;
end;

procedure TTMSFNCBarCode.ApplyTextFont(AGraphics: TTMSFNCGraphics);
begin
  {$IFDEF FMXLIB}
  AGraphics.Font.Assign(FTextFont);
  {$ELSE}
  AGraphics.Font.Assign(Font);
  {$ENDIF}
end;

function TTMSFNCBarCode.MeasureTextHeight(const AText: string): Integer;
var
  Graphics: TTMSFNCGraphics;
begin
  Result := 0;
  if AText = '' then
    Exit;

  Graphics := TTMSFNCGraphics.CreateBitmapCanvas(1, 1, False, False);
  try
    if Graphics.BeginScene then
    begin
      ApplyTextFont(Graphics);
      Result := Ceil(Graphics.CalculateTextHeight(AText));
      Graphics.EndScene;
    end;
  finally
    Graphics.Free;
  end;
end;

function TTMSFNCBarCode.EncodedPattern: string;
var
  Text: string;
begin
  if not TryEncode(Result, Text) then
    raise EConvertError.Create(FLastError);
end;

procedure TTMSFNCBarCode.Draw(AGraphics: TTMSFNCGraphics; ARect: TRectF);
begin
  inherited;
  DrawBarcode(AGraphics, ARect, True);
end;

procedure TTMSFNCBarCode.DrawBarcode(AGraphics: TTMSFNCGraphics;
  const ABounds: TRectF; AShowError: Boolean);
var
  BarHeight, BarcodeWidth, BarWidth, I, TextHeight: Integer;
  AvailableHeight, AvailableWidth, RenderHeight, RenderWidth: Integer;
  BoundsLeft, BoundsTop, BoundsRight, BoundsBottom: Integer;
  Pattern, TextToDraw: string;
  TextRect: TRectF;
  TextTop, X, Y: Integer;
begin
  FillGraphicsRect(AGraphics, ABounds, FBackgroundColor);

  if not TryEncode(Pattern, TextToDraw) then
  begin
    if AShowError then
    begin
      ApplyTextFont(AGraphics);
      AGraphics.Font.Color := gcRed;
      AGraphics.DrawText(ABounds, FLastError, True, gtaCenter, gtaCenter, gttWord);
    end
    else
      raise EConvertError.Create(FLastError);
    Exit;
  end;

  if Pattern = '' then
    Exit;

  BoundsLeft := Round(ABounds.Left);
  BoundsTop := Round(ABounds.Top);
  BoundsRight := Round(ABounds.Right);
  BoundsBottom := Round(ABounds.Bottom);
  RenderWidth := BoundsRight - BoundsLeft;
  RenderHeight := BoundsBottom - BoundsTop;
  if (RenderWidth <= 0) or (RenderHeight <= 0) then
    Exit;

  AvailableWidth := RenderWidth - FMargin * 2;
  AvailableHeight := RenderHeight - FMargin * 2;
  if (AvailableWidth <= 0) or (AvailableHeight <= 0) then
    Exit;

  ApplyTextFont(AGraphics);
  TextHeight := 0;
  if FDisplayValue and (TextToDraw <> '') then
    TextHeight := Ceil(AGraphics.CalculateTextHeight(TextToDraw)) + FTextMargin;

  Dec(AvailableHeight, TextHeight);
  if AvailableHeight <= 0 then
    Exit;

  BarWidth := FModuleWidth;
  if Length(Pattern) * BarWidth > AvailableWidth then
    BarWidth := AvailableWidth div Length(Pattern);
  if BarWidth < 1 then
    BarWidth := 1;

  BarcodeWidth := Length(Pattern) * BarWidth;
  X := BoundsLeft + FMargin + (AvailableWidth - BarcodeWidth) div 2;
  Y := BoundsTop + FMargin;

  BarHeight := FBarHeight;
  if BarHeight > AvailableHeight then
    BarHeight := AvailableHeight;
  if BarHeight <= 0 then
    Exit;

  for I := 1 to Length(Pattern) do
    if Pattern[I] = '1' then
      FillGraphicsRect(AGraphics, RectF(X + (I - 1) * BarWidth, Y,
        X + I * BarWidth, Y + BarHeight), FBarColor);

  if FDisplayValue and (TextToDraw <> '') then
  begin
    ApplyTextFont(AGraphics);
    TextTop := Y + BarHeight + FTextMargin;
    TextRect := RectF(BoundsLeft + FMargin, TextTop, BoundsRight - FMargin,
      BoundsBottom - FMargin);
    AGraphics.DrawText(TextRect, TextToDraw, False, gtaCenter, gtaLeading,
      gttCharacter);
  end;
end;

{$IFDEF FMXLIB}
procedure TTMSFNCBarCode.FontChanged(Sender: TObject);
begin
  Invalidate;
end;

procedure TTMSFNCBarCode.SetFont(const AValue: TTMSFNCGraphicsFont);
begin
  if Assigned(AValue) then
    FTextFont.Assign(AValue);
end;
{$ENDIF}

procedure TTMSFNCBarCode.RegisterRuntimeClasses;
begin
  inherited;
  RegisterClass(TTMSFNCBarCode);
end;

procedure TTMSFNCBarCode.SaveToPNG(const AFileName: string);
var
  ExportHeight, ExportWidth: Integer;
  Graphics: TTMSFNCGraphics;
begin
  ExportWidth := ExportDimension(Width, 320);
  ExportHeight := ExportDimension(Height, 140);

  Graphics := TTMSFNCGraphics.CreateBitmapCanvas(ExportWidth, ExportHeight,
    False, False);
  try
    if Graphics.BeginScene then
    begin
      DrawBarcode(Graphics, RectF(0, 0, ExportWidth, ExportHeight), False);
      Graphics.EndScene;
    end;
    SaveGraphicsBitmapToPNG(Graphics.Bitmap, AFileName);
  finally
    Graphics.Free;
  end;
end;

function SVGFontColor(AControl: TTMSFNCBarCode): string;
begin
  {$IFDEF FMXLIB}
  Result := SVGColor(AControl.FTextFont.Color);
  {$ELSE}
  Result := SVGColor(AControl.Font.Color);
  {$ENDIF}
end;

function SVGFontName(AControl: TTMSFNCBarCode): string;
begin
  {$IFDEF FMXLIB}
  Result := AControl.FTextFont.Name;
  {$ELSE}
  Result := AControl.Font.Name;
  {$ENDIF}
end;

function SVGFontSize(AControl: TTMSFNCBarCode): Integer;
begin
  {$IFDEF FMXLIB}
  Result := Round(AControl.FTextFont.Size);
  {$ELSE}
  Result := Abs(AControl.Font.Height);
  if Result <= 0 then
    Result := AControl.Font.Size;
  {$ENDIF}
  if Result <= 0 then
    Result := 12;
end;

function SVGFontStyle(AControl: TTMSFNCBarCode): string;
begin
  {$IFDEF FMXLIB}
  if TFontStyle.fsItalic in AControl.FTextFont.Style then
  {$ELSE}
  if fsItalic in AControl.Font.Style then
  {$ENDIF}
    Result := 'italic'
  else
    Result := 'normal';
end;

function SVGFontWeight(AControl: TTMSFNCBarCode): string;
begin
  {$IFDEF FMXLIB}
  if TFontStyle.fsBold in AControl.FTextFont.Style then
  {$ELSE}
  if fsBold in AControl.Font.Style then
  {$ENDIF}
    Result := 'bold'
  else
    Result := 'normal';
end;

function XMLText(const AValue: string): string;
begin
  Result := StringReplace(AValue, '&', '&amp;', [rfReplaceAll]);
  Result := StringReplace(Result, '<', '&lt;', [rfReplaceAll]);
  Result := StringReplace(Result, '>', '&gt;', [rfReplaceAll]);
  Result := StringReplace(Result, '"', '&quot;', [rfReplaceAll]);
  Result := StringReplace(Result, '''', '&apos;', [rfReplaceAll]);
end;

procedure TTMSFNCBarCode.SaveToSVG(const AFileName: string);
var
  AvailableHeight, AvailableWidth, BarHeight, BarcodeWidth, BarWidth: Integer;
  ExportHeight, ExportWidth, I, TextHeight, TextTop, X, Y: Integer;
  Pattern, TextToDraw: string;
  SVG: TStringList;
begin
  if not TryEncode(Pattern, TextToDraw) then
    raise EConvertError.Create(FLastError);

  ExportWidth := ExportDimension(Width, 320);
  ExportHeight := ExportDimension(Height, 140);

  SVG := TStringList.Create;
  try
    SVG.Add('<?xml version="1.0" encoding="UTF-8"?>');
    SVG.Add(Format(
      '<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">',
      [ExportWidth, ExportHeight, ExportWidth, ExportHeight]));
    SVG.Add(Format('  <rect width="100%%" height="100%%" fill="%s"/>',
      [SVGColor(FBackgroundColor)]));

    if Pattern <> '' then
    begin
      AvailableWidth := ExportWidth - FMargin * 2;
      AvailableHeight := ExportHeight - FMargin * 2;
      if (AvailableWidth > 0) and (AvailableHeight > 0) then
      begin
        TextHeight := 0;
        if FDisplayValue and (TextToDraw <> '') then
          TextHeight := MeasureTextHeight(TextToDraw) + FTextMargin;

        Dec(AvailableHeight, TextHeight);
        if AvailableHeight > 0 then
        begin
          BarWidth := FModuleWidth;
          if Length(Pattern) * BarWidth > AvailableWidth then
            BarWidth := AvailableWidth div Length(Pattern);
          if BarWidth < 1 then
            BarWidth := 1;

          BarcodeWidth := Length(Pattern) * BarWidth;
          X := FMargin + (AvailableWidth - BarcodeWidth) div 2;
          Y := FMargin;

          BarHeight := FBarHeight;
          if BarHeight > AvailableHeight then
            BarHeight := AvailableHeight;

          if BarHeight > 0 then
          begin
            for I := 1 to Length(Pattern) do
            begin
              if Pattern[I] = '1' then
                SVG.Add(Format(
                  '  <rect x="%d" y="%d" width="%d" height="%d" fill="%s"/>',
                  [X + (I - 1) * BarWidth, Y, BarWidth, BarHeight,
                   SVGColor(FBarColor)]));
            end;

            if FDisplayValue and (TextToDraw <> '') then
            begin
              TextTop := Y + BarHeight + FTextMargin;
              SVG.Add(Format(
                '  <text x="%d" y="%d" text-anchor="middle" dominant-baseline="text-before-edge" font-family="%s" font-size="%d" font-weight="%s" font-style="%s" fill="%s">%s</text>',
                [ExportWidth div 2, TextTop, XMLText(SVGFontName(Self)),
                 SVGFontSize(Self), SVGFontWeight(Self), SVGFontStyle(Self),
                 SVGFontColor(Self), XMLText(TextToDraw)]));
            end;
          end;
        end;
      end;
    end;

    SVG.Add('</svg>');
    SVG.SaveToFile(AFileName {$IFNDEF WEBLIB}, TEncoding.UTF8{$ENDIF});
  finally
    SVG.Free;
  end;
end;

procedure TTMSFNCBarCode.SetBackgroundColor(AValue: TTMSFNCGraphicsColor);
begin
  if FBackgroundColor <> AValue then
  begin
    FBackgroundColor := AValue;
    Color := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetBarcodeType(AValue: TTMSBarcodeType);
begin
  if FBarcodeType <> AValue then
  begin
    FBarcodeType := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetBarColor(AValue: TTMSFNCGraphicsColor);
begin
  if FBarColor <> AValue then
  begin
    FBarColor := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetBarHeight(AValue: Integer);
begin
  if AValue < 1 then
    AValue := 1;
  if FBarHeight <> AValue then
  begin
    FBarHeight := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetDisplayValue(AValue: Boolean);
begin
  if FDisplayValue <> AValue then
  begin
    FDisplayValue := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetEAN128(AValue: Boolean);
begin
  if FEAN128 <> AValue then
  begin
    FEAN128 := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetMargin(AValue: Integer);
begin
  if AValue < 0 then
    AValue := 0;
  if FMargin <> AValue then
  begin
    FMargin := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetMod43Checksum(AValue: Boolean);
begin
  if FMod43Checksum <> AValue then
  begin
    FMod43Checksum := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetModuleWidth(AValue: Integer);
begin
  if AValue < 1 then
    AValue := 1;
  if FModuleWidth <> AValue then
  begin
    FModuleWidth := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetTextMargin(AValue: Integer);
begin
  if AValue < 0 then
    AValue := 0;
  if FTextMargin <> AValue then
  begin
    FTextMargin := AValue;
    Invalidate;
  end;
end;

procedure TTMSFNCBarCode.SetValue(const AValue: string);
begin
  if FValue <> AValue then
  begin
    FValue := AValue;
    Invalidate;
  end;
end;

function TTMSFNCBarCode.TryEncode(out APattern, AText: string): Boolean;
begin
  Result := EncodeBarcodeValue(FBarcodeType, FValue, FMod43Checksum, FEAN128,
    APattern, AText, FLastError);
end;

procedure Register;
begin
  {$IFNDEF WEBLIB}
  RegisterComponents('TMS FNC UI', [TTMSFNCBarCode]);
  {$ENDIF}
end;

end.
