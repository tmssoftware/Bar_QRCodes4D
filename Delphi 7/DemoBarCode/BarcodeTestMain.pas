unit Unit1;

interface

uses
  SysUtils, Classes, Controls, Forms, StdCtrls,
  ExtCtrls, Dialogs, TMSBarcode;

type
  TForm1 = class(TForm)
  private
    FBarcode: TTMSBarCode;
    FDisplayValue: TCheckBox;
    FExport: TButton;
    FFormat: TComboBox;
    FValue: TEdit;
    procedure DisplayValueChanged(Sender: TObject);
    procedure ExportClicked(Sender: TObject);
    procedure FormatChanged(Sender: TObject);
    procedure ValueChanged(Sender: TObject);
    function SampleValue(AType: TTMSBarcodeType): string;
    procedure UpdateBarcode;
  public
    constructor Create(AOwner: TComponent); override;
  end;

var
  Form1: TForm1;

implementation

constructor TForm1.Create(AOwner: TComponent);
var
  BarcodeType: TTMSBarcodeType;
  FormatLabel, ValueLabel: TLabel;
  TopPanel: TPanel;
begin
  inherited CreateNew(AOwner);

  Caption := 'VCL TMS Barcode Test';
  Position := poScreenCenter;
  Width := 840;
  Height := 420;

  TopPanel := TPanel.Create(Self);
  TopPanel.Parent := Self;
  TopPanel.Align := alTop;
  TopPanel.Height := 72;
  TopPanel.BevelOuter := bvNone;

  FormatLabel := TLabel.Create(Self);
  FormatLabel.Parent := TopPanel;
  FormatLabel.Left := 12;
  FormatLabel.Top := 13;
  FormatLabel.Caption := 'Format';

  FFormat := TComboBox.Create(Self);
  FFormat.Parent := TopPanel;
  FFormat.Left := 70;
  FFormat.Top := 10;
  FFormat.Width := 190;
  FFormat.Style := csDropDownList;
  for BarcodeType := Low(TTMSBarcodeType) to High(TTMSBarcodeType) do
    FFormat.Items.Add(TMSBarcodeTypeName(BarcodeType));
  FFormat.ItemIndex := Ord(tbcCODE128);
  FFormat.OnChange := FormatChanged;

  ValueLabel := TLabel.Create(Self);
  ValueLabel.Parent := TopPanel;
  ValueLabel.Left := 280;
  ValueLabel.Top := 13;
  ValueLabel.Caption := 'Value';

  FValue := TEdit.Create(Self);
  FValue.Parent := TopPanel;
  FValue.Left := 324;
  FValue.Top := 10;
  FValue.Width := 250;
  FValue.OnChange := ValueChanged;

  FDisplayValue := TCheckBox.Create(Self);
  FDisplayValue.Parent := TopPanel;
  FDisplayValue.Left := 594;
  FDisplayValue.Top := 12;
  FDisplayValue.Width := 120;
  FDisplayValue.Caption := 'Display text';
  FDisplayValue.Checked := True;
  FDisplayValue.OnClick := DisplayValueChanged;

  FExport := TButton.Create(Self);
  FExport.Parent := TopPanel;
  FExport.Left := 724;
  FExport.Top := 9;
  FExport.Width := 84;
  FExport.Height := 25;
  FExport.Caption := 'Export';
  FExport.OnClick := ExportClicked;

  FBarcode := TTMSBarCode.Create(Self);
  FBarcode.Parent := Self;
  FBarcode.Align := alClient;
  FBarcode.BarcodeType := tbcCODE128;
  FBarcode.Value := SampleValue(tbcCODE128);
  FBarcode.Font.Size := 14;

  FValue.Text := FBarcode.Value;
end;

procedure TForm1.DisplayValueChanged(Sender: TObject);
begin
  UpdateBarcode;
end;

procedure TForm1.ExportClicked(Sender: TObject);
var
  AppFolder: string;
  BMPFileName: string;
begin
  UpdateBarcode;

  AppFolder := IncludeTrailingPathDelimiter(ExtractFilePath(ParamStr(0)));
  BMPFileName := AppFolder + 'barcode.bmp';

  try
    FBarcode.SaveToBMP(BMPFileName);
    ShowMessage(Format('Exported barcode to:%s%s', [sLineBreak, BMPFileName]));
  except
    on E: Exception do
      ShowMessage('Export failed: ' + E.Message);
  end;
end;

procedure TForm1.FormatChanged(Sender: TObject);
var
  BarcodeType: TTMSBarcodeType;
begin
  BarcodeType := TTMSBarcodeType(FFormat.ItemIndex);
  FValue.Text := SampleValue(BarcodeType);
  UpdateBarcode;
end;

function TForm1.SampleValue(AType: TTMSBarcodeType): string;
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

procedure TForm1.UpdateBarcode;
begin
  FBarcode.BarcodeType := TTMSBarcodeType(FFormat.ItemIndex);
  FBarcode.Value := FValue.Text;
  FBarcode.DisplayValue := FDisplayValue.Checked;
end;

procedure TForm1.ValueChanged(Sender: TObject);
begin
  UpdateBarcode;
end;

end.
