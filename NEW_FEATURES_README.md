# 新功能說明 (New Features Guide)

本次更新為 SDLMM+Babylon3D 專案添加了完整的 3D 模型、紋理和光源支援。

This update adds complete 3D model, texture, and light source support to the SDLMM+Babylon3D project.

## 快速開始 (Quick Start)

### 1. 使用現有範例 (Use Existing Examples)

```bash
# 編譯測試程式
make test_scene_json

# 運行測試
./test_scene_json

# 查看生成的 JSON
cat test_3d_scene.json
```

### 2. 載入 3D 場景 (Load 3D Scene)

```c
#include "scene_json.h"

// 載入場景（自動載入模型、紋理、光源）
Scene3D* scene = scene3d_load_from_json("example_3d_scene.json");

// 創建渲染裝置
Device* device = device(800, 600);

// 渲染
scene3d_render(scene, device);

// 清理
device_free(device);
scene3d_free(scene);
```

## 新功能列表 (New Features)

### ✅ OBJ 模型載入 (OBJ Model Loading)

- 完整的 Wavefront OBJ 格式支援
- 自動解析頂點、法線、紋理座標
- 範例：`cube.obj`

**JSON 使用**:
```json
{
  "models": [{
    "modelFile": "cube.obj",
    "position": [0, 0, 0]
  }]
}
```

### ✅ 紋理映射 (Texture Mapping)

- 支援 PNG、BMP、JPG 等格式
- 自動載入和應用到模型
- 完整的 UV 座標支援

**JSON 使用**:
```json
{
  "models": [{
    "modelFile": "cube.obj",
    "textureFile": "texture.png"
  }]
}
```

### ✅ 光源系統 (Lighting System)

- 可定義多個光源
- 位置、強度、顏色可調
- 自動計算漫反射光照

**JSON 使用**:
```json
{
  "lights": [{
    "position": [5, 5, 5],
    "intensity": 1.0,
    "color": 16777215
  }]
}
```

### ✅ 內嵌網格 (Inline Mesh)

- 無需外部檔案
- JSON 中直接定義幾何體
- 適合簡單模型

**JSON 使用**:
```json
{
  "models": [{
    "mesh": {
      "vertices": [
        {"coordinates": [-1, -1, -1], "normal": [0, 1, 0]}
      ],
      "faces": [[0, 1, 2]]
    }
  }]
}
```

## 範例檔案 (Example Files)

| 檔案 | 說明 |
|------|------|
| `cube.obj` | 範例立方體模型 |
| `example_3d_scene.json` | 使用外部 OBJ 和紋理 |
| `example_3d_scene_inline.json` | 使用內嵌網格定義 |
| `texture.png` | 範例紋理（需自行提供）|

## 詳細文檔 (Detailed Documentation)

1. **[JSON_SCENE_FORMAT.md](JSON_SCENE_FORMAT.md)** - JSON 格式完整說明
2. **[OBJ_LOADER_README.md](OBJ_LOADER_README.md)** - OBJ 載入器使用指南
3. **[TEXTURE_SUPPORT.md](TEXTURE_SUPPORT.md)** - 紋理支援詳細文檔
4. **[LIGHT_SOURCE_PROCESSING.md](LIGHT_SOURCE_PROCESSING.md)** - 光源處理說明
5. **[FINAL_SUMMARY.md](FINAL_SUMMARY.md)** - 完整實作總結

## 完整範例 (Complete Example)

```json
{
  "format": 1,
  "width": 800,
  "height": 600,
  "camera": {
    "position": [0, 0, 10],
    "target": [0, 0, 0]
  },
  "lights": [
    {
      "position": [5, 5, 5],
      "intensity": 1.0,
      "color": 16777215
    }
  ],
  "models": [
    {
      "modelFile": "cube.obj",
      "textureFile": "texture.png",
      "position": [0, 0, 0],
      "rotation": [0, 0, 0],
      "scale": [1, 1, 1]
    }
  ]
}
```

## 編譯和測試 (Build and Test)

```bash
# 編譯測試程式
make test_scene_json

# 運行測試
./test_scene_json

# 編譯場景查看器 (需要 SDL)
make scene_viewer

# 查看場景
./scene_viewer example_3d_scene.json
```

## 常見問題 (FAQ)

**Q: 如何創建自己的 OBJ 模型？**
A: 使用 Blender、Maya 等 3D 軟體，導出為 Wavefront OBJ 格式。

**Q: 支援哪些紋理格式？**
A: PNG、BMP、JPG 等 SDL_image 支援的格式。

**Q: 可以有多個光源嗎？**
A: JSON 中可定義多個光源，目前渲染器使用第一個光源。

**Q: 紋理是必需的嗎？**
A: 不是，textureFile 是可選的。沒有紋理時使用預設光照顏色。

## 技術支援 (Technical Support)

遇到問題？查看：
1. 各個 .md 文檔的疑難排解部分
2. test_scene_json.c 範例程式碼
3. 測試輸出的錯誤訊息

## 授權 (License)

遵循 SDLMM 專案的 BSD 3-Clause License。
