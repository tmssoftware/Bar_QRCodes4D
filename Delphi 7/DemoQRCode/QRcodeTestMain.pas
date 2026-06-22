unit Unit2;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics,
  Controls, Forms, Dialogs, TMSQRCode, StdCtrls, ExtCtrls;

type
  TForm1 = class(TForm)
  private
    { Private declarations }
    FExport: TButton;
    FValue: TEdit;
    FQRCode: TTMSQRCode;
    procedure ValueChanged(Sender: TObject);
    procedure ExportClicked(Sender: TObject);

  public
    { Public declarations }
    constructor Create(AOwner: TComponent); override;
  end;

var
  Form1: TForm1;

implementation

constructor TForm1.Create(AOwner: TComponent);
var
  ValueLabel: TLabel;
  TopPanel: TPanel;
begin
  inherited CreateNew(AOwner);

  Caption := 'VCL QR Barcode Test';
  Position := poScreenCenter;
  Width := 840;
  Height := 420;

  TopPanel := TPanel.Create(Self);
  TopPanel.Parent := Self;
  TopPanel.Align := alTop;
  TopPanel.Height := 72;
  TopPanel.BevelOuter := bvNone;

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

  FExport := TButton.Create(Self);
  FExport.Parent := TopPanel;
  FExport.Left := 724;
  FExport.Top := 9;
  FExport.Width := 84;
  FExport.Height := 25;
  FExport.Caption := 'Export';
  FExport.OnClick := ExportClicked;

  FQRcode := TTMSQRCode.Create(Self);
  FQRcode.Parent := Self;
  FQRcode.Align := alClient;
  FQRcode.Text := FValue.Text;
end;

procedure TForm1.ExportClicked(Sender: TObject);
var
  AppFolder: string;
  BMPFileName: string;
begin
  AppFolder := IncludeTrailingPathDelimiter(ExtractFilePath(ParamStr(0)));
  BMPFileName := AppFolder + 'qrcode.bmp';

  try
    FQRcode.SaveToBMP(BMPFileName);
    ShowMessage(Format('Exported barcode to:%s%s', [sLineBreak, BMPFileName]));
  except
    on E: Exception do
      ShowMessage('Export failed: ' + E.Message);
  end;

end;

procedure TForm1.ValueChanged(Sender: TObject);
begin
  FQRcode.Text := FValue.Text;
end;

end.
