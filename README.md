# DPX-EXTERN v0.5

Aplicativo de simulação FPS em C++17 para Windows 10/11 x64.

**Escopo:** demonstração independente de ESP 2D, HP/armadura e seleção visual de alvos simulados. Não se conecta ao Valorant nem lê memória de jogos.

## Windows (Visual Studio 2022)

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Executável: `build/Release/DPX-EXTERN.exe`.

O workflow GitHub Actions compila e publica um artefato ZIP na aba Actions. A compilação Windows precisa ser confirmada pelo resultado do workflow.

## Controles

F1 ESP, F2 HP, F3 Aim Guide, F4 visible only, setas para FOV e smooth, espaço pausa, R reinicia, Esc sai.
