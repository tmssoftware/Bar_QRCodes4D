unit TMSBarCode;

interface

uses
  Windows, SysUtils, Classes, Controls, Graphics,
  Types;

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

  TTMSBarCode = class(TGraphicControl)
  private
    FBackgroundColor: TColor;
    FBarcodeType: TTMSBarcodeType;
    FBarColor: TColor;
    FBarHeight: Integer;
    FDisplayValue: Boolean;
    FEAN128: Boolean;
    FLastError: string;
    FMargin: Integer;
    FMod43Checksum: Boolean;
    FModuleWidth: Integer;
    FTextMargin: Integer;
    FValue: string;
    procedure SetBackgroundColor(AValue: TColor);
    procedure SetBarcodeType(AValue: TTMSBarcodeType);
    procedure SetBarColor(AValue: TColor);
    procedure SetBarHeight(AValue: Integer);
    procedure SetDisplayValue(AValue: Boolean);
    procedure SetEAN128(AValue: Boolean);
    procedure SetMargin(AValue: Integer);
    procedure SetMod43Checksum(AValue: Boolean);
    procedure SetModuleWidth(AValue: Integer);
    procedure SetTextMargin(AValue: Integer);
    procedure SetValue(const AValue: string);
  protected
    procedure DrawBarcode(ACanvas: TCanvas; const ABounds: TRect;
      AShowError: Boolean);
    procedure Paint; override;
  public
    constructor Create(AOwner: TComponent); override;
    function EncodedPattern: string;
    procedure SaveToBMP(const AFileName: string);
    {
    procedure SaveToPNG(const AFileName: string);
    procedure SaveToSVG(const AFileName: string);
    }
    function TryEncode(out APattern, AText: string): Boolean;
    property LastError: string read FLastError;
  published
    property Align;
    property Anchors;
    property BackgroundColor: TColor read FBackgroundColor write SetBackgroundColor default clWhite;
    property BarcodeType: TTMSBarcodeType read FBarcodeType write SetBarcodeType default tbcCODE128;
    property BarColor: TColor read FBarColor write SetBarColor default clBlack;
    property BarHeight: Integer read FBarHeight write SetBarHeight default 100;
    property DisplayValue: Boolean read FDisplayValue write SetDisplayValue default True;
    property EAN128: Boolean read FEAN128 write SetEAN128 default False;
    property Enabled;
    property Font;
    property Margin: Integer read FMargin write SetMargin default 10;
    property Mod43Checksum: Boolean read FMod43Checksum write SetMod43Checksum default False;
    property ModuleWidth: Integer read FModuleWidth write SetModuleWidth default 2;
    property ParentFont;
    property ParentShowHint;
    property PopupMenu;
    property ShowHint;
    property TextMargin: Integer read FTextMargin write SetTextMargin default 2;
    property Value: string read FValue write SetValue;
    property Visible;
    property OnClick;
    property OnDblClick;
    property OnMouseDown;
    property OnMouseMove;
    property OnMouseUp;
  end;

function TMSBarcodeTypeName(AType: TTMSBarcodeType): string;
function TMSBarcodeTypeFromName(const AName: string; out AType: TTMSBarcodeType): Boolean;

procedure Register;

implementation

//uses
  //System.UITypes,
  //Vcl.Imaging.pngimage;

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
  {
  if Data = '' then
    Exit(Fail('CODE39 requires at least one character.', Error));
  }
  // CÓDIGO MODIFICADO (Compatible con Delphi 7)
  if Data = '' then
    begin
      Result := Fail('CODE39 requires at least one character.', Error);
      Exit;
    end;

  for I := 1 to Length(Data) do
  begin
    if Code39CharValue(Data[I]) < 0 then
      begin
        Result := Fail('CODE39 accepts digits, uppercase letters, space, and - . $ / + %.', Error);
        Exit;
      end;
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
    begin
      Result := Fail('CODE128 requires at least one character.', Error);
      Exit;
    end;

  StartIndex := Ord(Prepared[1]) - 105;
  CurrentSet := Code128SetByStartIndex(StartIndex);
  if CurrentSet < 0 then
    begin
      Result := Fail('The CODE128 encoding does not start with a start character.', Error);
      Exit;
    end;

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
      begin
        Result := Fail('CODE128 produced an invalid symbol index.', Error);
        Exit;
      end;

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
    begin
      Result := Fail('CODE128 requires at least one character.', Error);
      Exit;
    end;
  for I := 1 to Length(Data) do
  begin
    if not IsCode128Allowed(Data[I]) then
      begin
        Result := Fail('CODE128 accepts ASCII characters and JsBarcode special codes #200..#211.', Error);
        Exit;
      end;
  end;
  Result := True;
end;

function ValidateCode128Set(const Data: string; ASet: Integer; out Error: string): Boolean;
var
  I: Integer;
begin
  if Data = '' then
    begin
      Result := Fail('CODE128 requires at least one character.', Error);
      Exit;
    end;
  if ASet = CODE128_SET_C then
  begin
    if CountSetCLength(Data, 1) <> Length(Data) then
      begin
        Result := Fail('CODE128C requires digit pairs and optional FNC1 (#207).', Error);
        Exit;
      end;
  end
  else
  begin
    for I := 1 to Length(Data) do
    begin
      if (ASet = CODE128_SET_A) and not IsCode128SetAChar(Data[I]) then
        begin
          Result := Fail('CODE128A accepts ASCII #0..#95 and JsBarcode special codes #200..#207.', Error);
          Exit;
        end;
      if (ASet = CODE128_SET_B) and not IsCode128SetBChar(Data[I]) then
        begin
          Result := Fail('CODE128B accepts ASCII #32..#127 and JsBarcode special codes #200..#207.', Error);
          Exit;
        end;
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
          begin
            Result := False;
            Exit;
          end;
        Prepared := Chr(CODE128_START_A_CHAR) + Value;
      end;
    tbcCODE128B:
      begin
        if not ValidateCode128Set(Value, CODE128_SET_B, Error) then
          begin
            Result := False;
            Exit;
          end;
        Prepared := Chr(CODE128_START_B_CHAR) + Value;
      end;
    tbcCODE128C:
      begin
        if not ValidateCode128Set(Value, CODE128_SET_C, Error) then
          begin
            Result := False;
            Exit;
          end;
        Prepared := Chr(CODE128_START_C_CHAR) + Value;
      end;
  else
    if not ValidateCode128Prepared(Value, Error) then
      begin
            Result := False;
            Exit;
          end;
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
    begin
      Result := Fail('EAN13 requires 12 digits or 13 digits with a valid checksum.', Error);
      Exit;
    end;
  if CharToDigit(Data[13]) <> EAN13Checksum(Data) then
    begin
      Result := Fail('EAN13 requires 12 digits or 13 digits with a valid checksum.', Error);
      Exit;
    end;

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
    begin
      Result := Fail('EAN8 requires 7 digits or 8 digits with a valid checksum.', Error);
      Exit;
    end;
  if CharToDigit(Data[8]) <> EAN8Checksum(Data) then
    begin
      Result := Fail('EAN8 requires 7 digits or 8 digits with a valid checksum.', Error);
      Exit;
    end;

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
    begin
      Result := Fail('EAN5 requires exactly 5 digits.', Error);
      Exit;
    end;

  Pattern := '1011' + EANEncodeDigits(Value, EAN5_STRUCTURE[EAN5Checksum(Value)], '01');
  Text := Value;
  Result := True;
end;

function EncodeEAN2(const Value: string; out Pattern, Text, Error: string): Boolean;
var
  N: Integer;
begin
  if not IsDigits(Value, 2) then
    begin
      Result := Fail('EAN2 requires exactly 2 digits.', Error);
      Exit;
    end;

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
    begin
      Result := Fail('UPC requires 11 digits or 12 digits with a valid checksum.', Error);
      Exit;
    end;
  if CharToDigit(Data[12]) <> UPCChecksum(Data) then
    begin
      Result := Fail('UPC requires 11 digits or 12 digits with a valid checksum.', Error);
      Exit;
    end;

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
      begin
        Result := Fail('UPCE checksum does not match the expanded UPC-A value.', Error);
        Exit;
      end;
    Text := Value;
  end
  else
    begin
      Result := Fail('UPCE requires 6 digits or 8 digits beginning with 0 or 1.', Error);
      Exit;
    end;

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
      begin
        Result := Fail('ITF14 requires 13 digits or 14 digits with a valid checksum.', Error);
        Exit;
      end;
    if CharToDigit(Data[14]) <> ITF14Checksum(Data) then
      begin
        Result := Fail('ITF14 requires 13 digits or 14 digits with a valid checksum.', Error);
        Exit;
      end;
  end
  else
  begin
    if (not IsDigits(Data, -1)) or Odd(Length(Data)) then
      begin
        Result := Fail('ITF requires an even number of digits.', Error);
        Exit;
      end;
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
    begin
      Result := Fail('MSI requires one or more digits.', Error);
      Exit;
    end;

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
    begin
      Result := Fail('pharmacode requires a number from 3 through 131070.', Error);
      Exit;
    end;

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
    begin
      Result := Fail('codabar requires data.', Error);
      Exit;
    end;

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
      begin
        Result := Fail('codabar requires A-D start/end guards, or plain digits and - $ : . + /.', Error);
        Exit;
      end;

  for I := 2 to Length(Data) - 1 do
  begin
    if Pos(Data[I], '0123456789-$:/.+') = 0 then
      begin
        Result := Fail('codabar body accepts digits and - $ : . + /.', Error);
        Exit;
      end;
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
    begin
      Result := Fail('CODE93 requires at least one character.', Error);
      Exit;
    end;

  SetLength(Symbols, 0);
  for I := 1 to Length(Value) do
  begin
    if FullASCII then
    begin
      if (Ord(Value[I]) > 127) or
        (not Code93AddFullAsciiChar(Value[I], Symbols)) then
        begin
          Result := Fail('CODE93FullASCII accepts ASCII #0..#127.', Error);
          Exit;
        end;
    end
    else
    begin
      SymbolText := Value[I];
      if Code93SymbolValue(SymbolText) < 0 then
        begin
          Result := Fail('CODE93 accepts digits, uppercase letters, space, and - . $ / + %.', Error);
          Exit;
        end;
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

function ExportDimension(AValue, ADefault: Integer): Integer;
begin
  Result := AValue;
  if Result <= 0 then
    Result := ADefault;
end;

function MeasureTextHeight(AFont: TFont; const AText: string): Integer;
var
  Bitmap: TBitmap;
begin
  Result := 0;
  if AText = '' then
    Exit;

  Bitmap := TBitmap.Create;
  try
    Bitmap.Width := 1;
    Bitmap.Height := 1;
    Bitmap.Canvas.Font.Assign(AFont);
    Result := Bitmap.Canvas.TextHeight(AText);
  finally
    Bitmap.Free;
  end;
end;

function SVGColor(AColor: TColor): string;
var
  RGB: COLORREF;
begin
  RGB := ColorToRGB(AColor);
  Result := '#' + IntToHex(GetRValue(RGB), 2) +
    IntToHex(GetGValue(RGB), 2) + IntToHex(GetBValue(RGB), 2);
end;

function SVGFontSize(AFont: TFont): Integer;
begin
  Result := Abs(AFont.Height);
  if Result <= 0 then
    Result := AFont.Size;
  if Result <= 0 then
    Result := 12;
end;

function SVGFontStyle(AFont: TFont): string;
begin
  if fsItalic in AFont.Style then
    Result := 'italic'
  else
    Result := 'normal';
end;

function SVGFontWeight(AFont: TFont): string;
begin
  if fsBold in AFont.Style then
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

constructor TTMSBarCode.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  Width := 320;
  Height := 140;
  FBackgroundColor := clWhite;
  FBarcodeType := tbcCODE128;
  FBarColor := clBlack;
  FBarHeight := 100;
  FDisplayValue := True;
  FEAN128 := False;
  FMargin := 10;
  FMod43Checksum := False;
  FModuleWidth := 2;
  FTextMargin := 2;
  FValue := '123456789012';
  Font.Name := 'Consolas';
  Font.Size := 12;
end;

function TTMSBarCode.EncodedPattern: string;
var
  Text: string;
begin
  if not TryEncode(Result, Text) then
    raise EConvertError.Create(FLastError);
end;

procedure TTMSBarCode.DrawBarcode(ACanvas: TCanvas; const ABounds: TRect;
  AShowError: Boolean);
var
  BarHeight, BarcodeWidth, BarWidth, I, TextHeight, TextTop, X, Y: Integer;
  AvailableHeight, AvailableWidth, RenderHeight, RenderWidth: Integer;
  Pattern, TextToDraw: string;
  TextRect: TRect;
begin
  ACanvas.Brush.Style := bsSolid;
  ACanvas.Brush.Color := FBackgroundColor;
  ACanvas.FillRect(ABounds);

  if not TryEncode(Pattern, TextToDraw) then
  begin
    if AShowError then
    begin
      ACanvas.Font.Assign(Font);
      ACanvas.Font.Color := clRed;
      ACanvas.Brush.Style := bsClear;
      TextRect := ABounds;
      DrawText(ACanvas.Handle, PChar(FLastError), -1, TextRect,
        DT_CENTER or DT_VCENTER or DT_WORDBREAK);
      ACanvas.Brush.Style := bsSolid;
    end
    else
      raise EConvertError.Create(FLastError);
    Exit;
  end;

  if Pattern = '' then
    Exit;

  RenderWidth := ABounds.Right - ABounds.Left;
  RenderHeight := ABounds.Bottom - ABounds.Top;
  if (RenderWidth <= 0) or (RenderHeight <= 0) then
    Exit;

  AvailableWidth := RenderWidth - FMargin * 2;
  AvailableHeight := RenderHeight - FMargin * 2;
  if (AvailableWidth <= 0) or (AvailableHeight <= 0) then
    Exit;

  ACanvas.Font.Assign(Font);
  TextHeight := 0;
  if FDisplayValue and (TextToDraw <> '') then
    TextHeight := ACanvas.TextHeight(TextToDraw) + FTextMargin;

  Dec(AvailableHeight, TextHeight);
  if AvailableHeight <= 0 then
    Exit;

  BarWidth := FModuleWidth;
  if Length(Pattern) * BarWidth > AvailableWidth then
    BarWidth := AvailableWidth div Length(Pattern);
  if BarWidth < 1 then
    BarWidth := 1;

  BarcodeWidth := Length(Pattern) * BarWidth;
  X := ABounds.Left + FMargin + (AvailableWidth - BarcodeWidth) div 2;
  Y := ABounds.Top + FMargin;

  BarHeight := FBarHeight;
  if BarHeight > AvailableHeight then
    BarHeight := AvailableHeight;
  if BarHeight <= 0 then
    Exit;

  ACanvas.Brush.Color := FBarColor;
  for I := 1 to Length(Pattern) do
  begin
    if Pattern[I] = '1' then
      ACanvas.FillRect(Rect(X + (I - 1) * BarWidth, Y,
        X + I * BarWidth, Y + BarHeight));
  end;

  if FDisplayValue and (TextToDraw <> '') then
  begin
    ACanvas.Font.Assign(Font);
    ACanvas.Brush.Style := bsClear;
    TextTop := Y + BarHeight + FTextMargin;
    TextRect := Rect(ABounds.Left + FMargin, TextTop, ABounds.Right - FMargin,
      ABounds.Bottom - FMargin);
    DrawText(ACanvas.Handle, PChar(TextToDraw), -1, TextRect,
      DT_CENTER or DT_TOP or DT_SINGLELINE or DT_END_ELLIPSIS);
    ACanvas.Brush.Style := bsSolid;
  end;
end;

procedure TTMSBarCode.Paint;
begin
  DrawBarcode(Canvas, ClientRect, True);
end;

procedure TTMSBarCode.SaveToBMP(const AFileName: string);
var
  BMP: TBitmap;
begin
  BMP := TBitmap.Create;
  try
    // Configurar dimensiones
    BMP.Width := Width;
    BMP.Height := Height;
    
    // Pintar el fondo
    BMP.Canvas.Brush.Color := FBackgroundColor;
    BMP.Canvas.FillRect(Rect(0, 0, Width, Height));
    
    // Dibujar el código de barras
    DrawBarcode(BMP.Canvas, Rect(0, 0, Width, Height), False);
    
    // Guardar como mapa de bits (.bmp)
    BMP.SaveToFile(AFileName);
  finally
    BMP.Free;
  end;
end;
{
procedure TTMSBarCode.SaveToPNG(const AFileName: string);
var
  Bitmap: TBitmap;
  ExportHeight, ExportWidth: Integer;
  PNG: TPngImage;
begin
  ExportWidth := ExportDimension(Width, 320);
  ExportHeight := ExportDimension(Height, 140);

  Bitmap := TBitmap.Create;
  PNG := TPngImage.Create;
  try
    Bitmap.PixelFormat := pf32bit;
    Bitmap.Width := ExportWidth;
    Bitmap.Height := ExportHeight;
    DrawBarcode(Bitmap.Canvas, Rect(0, 0, ExportWidth, ExportHeight), False);
    PNG.Assign(Bitmap);
    PNG.SaveToFile(AFileName);
  finally
    PNG.Free;
    Bitmap.Free;
  end;
end;

procedure TTMSBarCode.SaveToSVG(const AFileName: string);
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
          TextHeight := MeasureTextHeight(Font, TextToDraw) + FTextMargin;

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
                [ExportWidth div 2, TextTop, XMLText(Font.Name),
                 SVGFontSize(Font), SVGFontWeight(Font), SVGFontStyle(Font),
                 SVGColor(Font.Color), XMLText(TextToDraw)]));
            end;
          end;
        end;
      end;
    end;

    SVG.Add('</svg>');
    SVG.SaveToFile(AFileName, TEncoding.UTF8);
  finally
    SVG.Free;
  end;
end;
}
procedure TTMSBarCode.SetBackgroundColor(AValue: TColor);
begin
  if FBackgroundColor <> AValue then
  begin
    FBackgroundColor := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetBarcodeType(AValue: TTMSBarcodeType);
begin
  if FBarcodeType <> AValue then
  begin
    FBarcodeType := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetBarColor(AValue: TColor);
begin
  if FBarColor <> AValue then
  begin
    FBarColor := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetBarHeight(AValue: Integer);
begin
  if AValue < 1 then
    AValue := 1;
  if FBarHeight <> AValue then
  begin
    FBarHeight := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetDisplayValue(AValue: Boolean);
begin
  if FDisplayValue <> AValue then
  begin
    FDisplayValue := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetEAN128(AValue: Boolean);
begin
  if FEAN128 <> AValue then
  begin
    FEAN128 := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetMargin(AValue: Integer);
begin
  if AValue < 0 then
    AValue := 0;
  if FMargin <> AValue then
  begin
    FMargin := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetMod43Checksum(AValue: Boolean);
begin
  if FMod43Checksum <> AValue then
  begin
    FMod43Checksum := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetModuleWidth(AValue: Integer);
begin
  if AValue < 1 then
    AValue := 1;
  if FModuleWidth <> AValue then
  begin
    FModuleWidth := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetTextMargin(AValue: Integer);
begin
  if AValue < 0 then
    AValue := 0;
  if FTextMargin <> AValue then
  begin
    FTextMargin := AValue;
    Invalidate;
  end;
end;

procedure TTMSBarCode.SetValue(const AValue: string);
begin
  if FValue <> AValue then
  begin
    FValue := AValue;
    Invalidate;
  end;
end;

function TTMSBarCode.TryEncode(out APattern, AText: string): Boolean;
begin
  Result := EncodeBarcodeValue(FBarcodeType, FValue, FMod43Checksum, FEAN128,
    APattern, AText, FLastError);
end;

procedure Register;
begin
  RegisterComponents('TMS Bar && QR Codes', [TTMSBarCode]);
end;

end.
