# Printing from Claude Code (Windows)

Methods for printing files from Claude Code terminal on Windows.

## Methods

### Method 1: PowerShell Print Verb (Recommended)
```powershell
powershell -Command "Start-Process 'C:/path/to/file.txt' -Verb Print"
```
Opens the default application for the file type and sends to default printer.

### Method 2: Notepad Print
```cmd
notepad /p "C:/path/to/file.txt"
```
Prints directly using Notepad (text files only).

### Method 3: Open and Print Manually
```cmd
notepad "C:/path/to/file.txt"
```
Opens file in Notepad, then user presses Ctrl+P to print.

## Troubleshooting

### Print command runs but nothing prints

**Possible causes:**
1. **WiFi printers** - Commands from CLI may not have full access to Windows print subsystem for network printers
2. **No default printer set** - Check Settings → Bluetooth & devices → Printers & scanners
3. **Printer offline/not connected**

**Solution:**
Open the file manually and use Ctrl+P:
1. Claude opens file with: `notepad "C:/path/to/file.txt"`
2. User presses Ctrl+P in Notepad
3. Select printer and print

### Checking available printers
```powershell
powershell -Command "Get-Printer | Select-Object Name, PrinterStatus"
```

### Print to specific printer
```powershell
powershell -Command "Get-Content 'file.txt' | Out-Printer -Name 'Printer Name'"
```

## Notes

- WiFi/network printers often require GUI interaction
- USB-connected printers work more reliably from command line
- For reliable printing, opening the file and letting user Ctrl+P is most consistent
