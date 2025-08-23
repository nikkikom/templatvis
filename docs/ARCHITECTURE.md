# Architecture

- Single template class `demo::Vec<T>` in `include/demo/vec.hpp`.
- One instantiation TU `src/vec.cpp` drives explicit instantiations (single-line extern/instantiate controlled by macros).
- Three cases are produced by compiling with different `DEMO_EXPORT_ON_*` flags via `add_case(...)`:
  - template_export: export on class template.
  - inst_export: export on explicit instantiation only.
  - all_export: export everywhere.
- throwers_*: simple factories returning `Base*` (new + dynamic_cast demos).
- main_* link both throwers_* and demo_* to ensure RTTI symbols are resolved on macOS.

## macOS и LC_UUID

Особенность `ld64`: всегда вставляет LC_UUID (рандомный UUID) в Mach-O.
Это мешает детерминизму.

Мы пробовали глобально убирать его (`-Wl,-no_uuid`), но `ld64` потом отказывался
линковать другие библиотеки, собранные без UUID.
Итоговое решение:

- По умолчанию — **не трогаем UUID**.
- Для экспериментов можно включить `APPLE_NO_UUID_FOR_EXE=ON`.
  В этом режиме `LC_UUID` убирается только у исполняемых файлов (main_*), но не у библиотек.
