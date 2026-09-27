# Style 


## Naming Convention
╔══════════════════════════════════════════════════════════════════════════╗
║	NAMING CONVENTION				                                       ║
╚══════════════════════════════════════════════════════════════════════════╝    
┌──────────────────────────────────────────────────────────────────────────┐
│ Element			┆ Style				┆ Example                          │
│┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄│
│ Class				┆ PascalCase		┆ ModelViewer, ShaderProgram	   │
│ Function			┆ camelCase()		┆ drawModel(), loadShader()		   │
│ Member Variable	┆ _camelCase		┆ _camera, _pbrShader			   │
│ Parameter			┆ camelCase			┆ setCamera(const Camera& camera)  │
│ Local Variable    ┆ camelCase			┆ modelMatrix, shaderProgram       │
│ Constants			┆ UPPER_CASE		┆ PI, 							   │
│ Enums				┆ PascalCase		┆ RenderMode::Wireframe			   │
└──────────────────────────────────────────────────────────────────────────┘



## Status Symbols

🔴 Yapılmadı / Başlanmadı
🟡 Devam Ediyor / Yapılıyor
🔶 Kısmen Yapıldı
🟢 Tamamlandı
⚪ Beklemede / Askıda
🟣 İnceleme / Test Aşamasında


## Comment Title

// ==========================
// Title
// ==========================