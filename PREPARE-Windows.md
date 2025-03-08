# Я тут буду конспектувати шо де я качав, шо ставив при встановлені оточення на Windows.

В цілому все по документації.

Відкриваємо powerShell від адміністратора.

```powershell

Get-ExecutionPolicy

# If it returns Restricted, then run

Set-ExecutionPolicy AllSigned

Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

choco feature enable -n allowGlobalConfirmation

choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'

choco install ninja gperf python311 git dtc-msys2 wget 7zip strawberryperl
```

Але я б python так не ставив би. Особисто я, використаю `uv`:

```powershell
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
```

Відкриваю окремий PowerShell з правами адміністратора та виконую (вже, мабуть, не треба):

```powershell
Get-ExecutionPolicy
```

Якшо отримали `Restricted`, виконуємо:

```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Ще довелось вирубати Alias

```powershell
Remove-Item Alias:wget
```

Качаю, встановлюю [VSCodeInsiders](https://code.visualstudio.com/docs/?dv=win&build=insiders)

Качаю, встановлюю [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) заради драйверів.

## Ініціалізація проєкта

Далі перевідкриваю PowerShell вже від користувача.

```powershell
cd ~\SDK
git clone https://github.com/FenixUkraine/fenix-fx500-hello-world.git
cd fenix-fx500-hello-world
```

Далі, відкриваю VSCode, в ньому відкриваю папку `~\SDK\fenix-fx500-hello-world` як проєкт, і далі вже там відкриваю вбудований powershell, і далі вся робота в ньому.

Рекомендую встановити доповнення `Serial Monitor`, дивитись логі прям з VSCode.

```powershell
code-insiders.cmd --install-extension ms-vscode.vscode-serial-monitor
```

Ну і більшічть шо VSCode сам буде пропонувати, або не чекаючи:

```powershell
code-insiders.cmd --install-extensionms-vscode.cmake-tools
code-insiders.cmd --install-extensionms-vscode.cpptools
code-insiders.cmd --install-extensionms-vscode.cpptools-extension-pack
code-insiders.cmd --install-extensionms-vscode.cpptools-themes
```


```powershell
uv venv .venv
.venv\Scripts\activate
uv pip install west
west init -l app
west update -o=--depth=1 -n
uv pip install pip
west packages pip --install
west sdk install
```

Дивимось чи є у `pyocd` підтримка M2354

```powershell
pyocd list --targets

...
m2354kjfae                Nuvoton                  M2354KJFAE                              builtin
...
```

Якшо немає, то треба встановити модуль, але скоріш за все
буде встановлений pyocd з вже вбудованою підтримкою m2354.

```powershell
pyocd pack find m2354

...
M2354SJFAE   Nuvoton   Nuvoton.NuMicro_DFP   1.3.26    False
...

pyocd pack install M2354SJFAE
```

Китайський ST-Link під Mac працював, а от під Windows чомусь відмовляється

```powershell
pyocd list
```

його не бачить.

Під STMCubeProgrammer він якось коряво розпізнається.

Пробую WCH Link, його pyocd бачить. І навіть ніякі драйвери не знадобились. Ще й USB-UART в комплекті.

```powershell
pyocd list

#  0   wch.cn WCH CMSIS-DAP   0001A0000001   n/a
```
