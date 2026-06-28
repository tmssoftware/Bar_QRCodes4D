unit Umainform;

interface

uses
  System.SysUtils, System.Classes, JS, Web, WEBLib.Graphics, WEBLib.Controls,
  WEBLib.Forms, WEBLib.Dialogs, VCL.TMSFNCTypes, VCL.TMSFNCUtils,
  VCL.TMSFNCGraphics, VCL.TMSFNCGraphicsTypes, Vcl.StdCtrls, WEBLib.StdCtrls,
  Vcl.Controls, WEBLib.ExtCtrls, VCL.TMSFNCCustomControl, VCL.TMSFNCQRCode,
  WEBLib.ComCtrls, VCL.TMSFNCBarCode, Vcl.Imaging.pngimage;

type
  TForm1 = class(TWebForm)
    WebPageControl1: TWebPageControl;
    WebPageControl1Sheet1: TWebTabSheet;
    WebPageControl1Sheet2: TWebTabSheet;
    TMSFNCQRCode1: TTMSFNCQRCode;
    WebPanel1: TWebPanel;
    WebPanel2: TWebPanel;
    WebLinkLabel1: TWebLinkLabel;
    WebEdit1: TWebEdit;
    WebLabel1: TWebLabel;
    WebButton1: TWebButton;
    WebButton2: TWebButton;
    TMSFNCBarCode1: TTMSFNCBarCode;
    WebComboBox1: TWebComboBox;
    WebEdit2: TWebEdit;
    WebLabel2: TWebLabel;
    WebLabel3: TWebLabel;
    WebImageControl1: TWebImageControl;
    WebLabel4: TWebLabel;
    procedure WebEdit1Change(Sender: TObject);
    procedure WebButton1Click(Sender: TObject);
    procedure WebButton2Click(Sender: TObject);
    procedure WebEdit2Change(Sender: TObject);
    procedure WebFormCreate(Sender: TObject);
    procedure WebComboBox1Change(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

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

function SampleValue(AType: TTMSBarcodeType): string;
begin
  case AType of
    tbcCODE39:
      Result := 'HELLO-39';
    tbcCODE128:
      Result := 'TMS Barcode 128';
    tbcCODE128A:
      Result := 'ABC123';
    tbcCODE128B:
      Result := 'Hello 128B';
    tbcCODE128C:
      Result := '12345678';
    tbcEAN13:
      Result := '590123412345';
    tbcEAN8:
      Result := '9638507';
    tbcEAN5:
      Result := '51234';
    tbcEAN2:
      Result := '12';
    tbcUPC:
      Result := '03600029145';
    tbcUPCE:
      Result := '123450';
    tbcITF14:
      Result := '1234567890123';
    tbcITF:
      Result := '123456';
    tbcMSI, tbcMSI10, tbcMSI11, tbcMSI1010, tbcMSI1110:
      Result := '123456';
    tbcPharmacode:
      Result := '12345';
    tbcCodabar:
      Result := '12345';
    tbcCODE93:
      Result := 'CODE93';
    tbcCODE93FullASCII:
      Result := 'Code93 full!';
    tbcGenericBarcode:
      Result := 'anything';
  else
    Result := '123456';
  end;
end;



procedure TForm1.WebButton1Click(Sender: TObject);
begin
  if WebPageControl1.ActivePageIndex = 0 then
  begin
    TMSFNCQRCode1.SaveToSVG('qrcode.svg');

  end;

  if WebPageControl1.ActivePageIndex = 1 then
  begin
    TMSFNCBarCode1.SaveToSVG('barcode.svg');
  end;
end;

procedure TForm1.WebButton2Click(Sender: TObject);
begin
  if WebPageControl1.ActivePageIndex = 0 then
  begin
    TMSFNCQRCode1.SaveToPNG('qrcode.png');
  end;

  if WebPageControl1.ActivePageIndex = 1 then
  begin
    TMSFNCBarCode1.SaveToPNG('barcode.png');
  end;
end;

procedure TForm1.WebComboBox1Change(Sender: TObject);
var
  BarcodeType: TTMSBarcodeType;
begin
  BarcodeType := TTMSBarcodeType(WebComboBox1.ItemIndex);
  TMSFNCBarCode1.Value := SampleValue(BarcodeType);
  WebEdit2.Text := TMSFNCBarCode1.Value;
end;

procedure TForm1.WebEdit1Change(Sender: TObject);
begin
  TMSFNCQRCode1.Text := WebEdit1.Text;
end;

procedure TForm1.WebEdit2Change(Sender: TObject);
begin
  TMSFNCBarCode1.Value := WebEdit2.Text;
end;

procedure TForm1.WebFormCreate(Sender: TObject);
var
  BarcodeType: TTMSBarcodeType;
begin
  for BarcodeType := Low(TTMSBarcodeType) to High(TTMSBarcodeType) do
    WebComboBox1.Items.Add(TMSBarcodeTypeName(BarcodeType));

  WebComboBox1.ItemIndex := 1;
  WebPageControl1.ActivePageIndex := 0;
end;

end.