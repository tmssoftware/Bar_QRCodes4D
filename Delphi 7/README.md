# TMS BarCode & QRCode Components for Delphi 7

This folder contains a backport of the `TMSBarCode` and `TMSQRCode` components, refactored specifically to be 100% compatible with **Delphi 7**. 

All modern framework dependencies (such as Unicode-specific `TEncoding`, `System.*` namespaces, and external PNG/SVG rendering engines) have been removed or adapted to use the native, standard VCL features available in Delphi 7.

---

> [!NOTE]
> **Key Changes for Delphi 7 Compatibility**
> - **Native BMP Export Only**: `SaveToPNG` and `SaveToSVG` methods have been removed or replaced with `SaveToBMP`. BMP format ensures 100% sharp pixel rendering, which is ideal for physical barcode/QR scanners.
> - **Removed Modern Namespaces**: Scopes like `Winapi.`, `System.`, and `Vcl.` were removed from the `uses` clause so the Delphi 7 compiler can find the files properly.
> - **Custom `TBytes` Type**: Implemented a local `TBytes = array of Byte` definition to preserve core data processing logic without breaking compliance.
> - **String Handling**: Replaced modern `TEncoding.UTF8` logic with native `UTF8Encode()` and string block manipulation.

---

## 📂 Demos & Validation

To help you test and validate the components immediately, we have included two fully functional testing applications just like the ones in the original modern version. You can open, compile, and run them directly in Delphi 7:

* **`DemoBarCode/`**: Contains the testing interface for `TTMSBarCode`. It allows you to select different encoding formats (Code39, Code128, EAN, etc.), customize the displayed value, and export the output to a `.bmp` file.
* **`DemoQRCode/`**: Contains the testing interface for `TTMSQRCode`. It lets you input live text, adjust sizing, and export the generated QR Code to a `.bmp` file.

> [!TIP]
> Running these demos is the easiest way to verify that the core barcode and QR generation algorithms work exactly like the original version before integrating the units into your own legacy projects.

---

## 🛠️ Installation

1. Copy the source files (`TMSBarCode.pas` and `TMSQRCode.pas`) into your Delphi 7 project folder or a dedicated library directory.
2. In Delphi 7, go to **Tools** > **Environment Options** > **Library** and add the directory path to your **Library Path**.
3. To install them into the IDE Palette:
   - Go to **Component** > **Install Component**.
   - Select `TMSBarCode.pas` or `TMSQRCode.pas`.
   - Choose a package (e.g., `dclusr.dpk`) and click **Compile** and **Install**.
   - The components will appear under a new palette tab named **`TMS Bar & QR Codes`**.

---

## 💻 Basic Usage Examples

### 1. Generating a QR Code dynamically
```pascal
uses TMSQRCode, Graphics;

procedure GenerateMyQR;
var
  QRCode: TTMSQRCode;
begin
  QRCode := TTMSQRCode.Create(Self);
  try
    QRCode.Width := 250;
    QRCode.Height := 250;
    QRCode.Text := '[https://github.com/tmssoftware/Bar_QRCodes4D](https://github.com/tmssoftware/Bar_QRCodes4D)';
    QRCode.DarkColor := clBlack;
    QRCode.LightColor := clWhite;
    
    // Save the QR Code to a standard bitmap file
    QRCode.SaveToBMP('C:\MyFolder\QRCode.bmp');
  finally
    QRCode.Free;
  end;
end;
```

### 2. Generating a Barcode dynamically
```pascal
uses TMSBarCode, Graphics;

procedure GenerateMyBarcode;
var
  Barcode: TTMSBarCode;
begin
  Barcode := TTMSBarCode.Create(Self);
  try
    Barcode.Width := 300;
    Barcode.Height := 100;
    Barcode.BarcodeType := tbcCODE128; // Standard Code 128
    Barcode.Value := 'TMS-DELPHI7-123';
    Barcode.DisplayValue := True; // Shows the text under the bars
    
    // Save the Barcode to a standard bitmap file
    Barcode.SaveToBMP('C:\MyFolder\Barcode.bmp');
  finally
    Barcode.Free;
  end;
end;
```
> [!IMPORTANT]
> To use these components visually, you can also just drop a TTMSBarCode or TTMSQRCode directly onto a standard TForm from the Component Palette after successful installation.
