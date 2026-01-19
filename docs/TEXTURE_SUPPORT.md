# 紋理支援說明 (Texture Support Documentation)

## 概述 (Overview)

3D 場景 JSON 格式現在完全支援紋理貼圖。每個模型都可以指定一個紋理檔案，該紋理將在渲染時自動載入並應用到模型上。

The 3D scene JSON format now fully supports texture mapping. Each model can specify a texture file that will be automatically loaded and applied to the model during rendering.

## JSON 格式 (JSON Format)

### 使用外部 OBJ 模型檔案 (Using External OBJ Model Files)

```json
{
  "format": 1,
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

### 使用內嵌網格資料 (Using Inline Mesh Data)

```json
{
  "format": 1,
  "models": [
    {
      "textureFile": "texture.png",
      "position": [0, 0, 0],
      "rotation": [0, 0, 0],
      "scale": [1, 1, 1],
      "mesh": {
        "vertices": [
          {"coordinates": [-1, -1, -1], "normal": [0, 0, -1], "texCoord": [0, 0]},
          {"coordinates": [ 1, -1, -1], "normal": [0, 0, -1], "texCoord": [1, 0]},
          {"coordinates": [ 1,  1, -1], "normal": [0, 0, -1], "texCoord": [1, 1]},
          {"coordinates": [-1,  1, -1], "normal": [0, 0, -1], "texCoord": [0, 1]}
        ],
        "faces": [[0, 1, 2], [0, 2, 3]]
      }
    }
  ]
}
```

## 屬性說明 (Property Description)

### textureFile

- **類型 (Type)**: 字串 (string)
- **必需 (Required)**: 否 (No) - 可選屬性
- **描述 (Description)**: 紋理圖片檔案的路徑
- **支援格式 (Supported Formats)**: PNG, BMP, JPG 等 SDL_image 支援的格式
- **範例 (Example)**: `"textureFile": "texture.png"`

## 紋理座標 (Texture Coordinates)

紋理座標 (UV 座標) 定義了如何將 2D 紋理貼圖映射到 3D 模型表面。

Texture coordinates (UV coordinates) define how a 2D texture image maps onto the 3D model surface.

### UV 座標系統 (UV Coordinate System)

- **U 軸**: 水平方向，範圍 0.0 到 1.0 (0 = 左邊, 1 = 右邊)
- **V 軸**: 垂直方向，範圍 0.0 到 1.0 (0 = 頂部, 1 = 底部)

### 在 OBJ 檔案中 (In OBJ Files)

OBJ 檔案使用 `vt` 指令定義紋理座標：

```obj
vt 0.0 0.0
vt 1.0 0.0
vt 1.0 1.0
vt 0.0 1.0
```

### 在 JSON 中 (In JSON)

JSON 內嵌網格使用 `texCoord` 屬性：

```json
{"coordinates": [-1, -1, -1], "texCoord": [0, 0]}
```

## 載入流程 (Loading Process)

1. 解析 JSON 場景檔案
2. 讀取 `modelFile` 或 `mesh` 資料
3. 如果指定了 `textureFile`，使用 `texture_load()` 載入紋理
4. 將紋理資料複製到 mesh 結構的 texture 欄位
5. 渲染時使用紋理座標進行紋理映射

## 使用範例 (Usage Examples)

### 範例 1: 基本立方體帶紋理 (Basic Textured Cube)

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
      "textureFile": "wood.png",
      "position": [0, 0, 0],
      "rotation": [0, 0, 0],
      "scale": [1, 1, 1]
    }
  ]
}
```

### 範例 2: 多個模型使用不同紋理 (Multiple Models with Different Textures)

```json
{
  "models": [
    {
      "modelFile": "cube.obj",
      "textureFile": "wood.png",
      "position": [-2, 0, 0]
    },
    {
      "modelFile": "sphere.obj",
      "textureFile": "metal.png",
      "position": [2, 0, 0]
    }
  ]
}
```

### 範例 3: 沒有紋理的模型 (Model Without Texture)

```json
{
  "models": [
    {
      "modelFile": "cube.obj",
      "position": [0, 0, 0]
    }
  ]
}
```

紋理是可選的。如果不指定 `textureFile`，模型將使用預設的白色或光照顏色渲染。

## 程式化使用 (Programmatic Usage)

### C API

```c
#include "scene_json.h"

// 創建模型並指定紋理
Model3D model = {0};
model.modelFile = "cube.obj";
model.textureFile = "texture.png";
model.position = vector3(0.0f, 0.0f, 0.0f);
model.rotation = vector3(0.0f, 0.0f, 0.0f);
model.scale = vector3(1.0f, 1.0f, 1.0f);

// 添加到場景
Scene3D* scene = scene3d_create(800, 600);
scene3d_add_model(scene, &model);

// 保存到 JSON（會包含 textureFile）
scene3d_save_to_json(scene, "scene.json");

// 載入場景（會自動載入紋理）
Scene3D* loaded = scene3d_load_from_json("scene.json");
```

## 技術細節 (Technical Details)

### 結構變更 (Structure Changes)

**Model3D 結構 (scene_json.h)**:
```c
typedef struct {
    Mesh* mesh;
    char* modelFile;
    char* textureFile;  // 新增 (New)
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
} Model3D;
```

### 記憶體管理 (Memory Management)

- `textureFile` 字串會自動複製和釋放
- 紋理資料會複製到 mesh 結構中
- 場景釋放時會自動清理所有資源

### 錯誤處理 (Error Handling)

- 如果紋理檔案不存在，會顯示警告但不會中斷載入
- 模型仍會被載入，只是沒有紋理
- 所有錯誤都會記錄到標準輸出

## 支援的圖片格式 (Supported Image Formats)

通過 SDL_image 函式庫支援：

- **PNG**: 推薦使用，支援透明度
- **BMP**: 基本格式，不支援透明度
- **JPG/JPEG**: 壓縮格式，適合照片紋理
- **TGA**: 遊戲常用格式
- **GIF**: 支援但不推薦用於紋理

## 最佳實踐 (Best Practices)

1. **紋理大小**: 使用 2 的冪次方尺寸 (如 256x256, 512x512, 1024x1024)
2. **格式選擇**: 需要透明度用 PNG，否則用 JPG 節省空間
3. **UV 映射**: 確保模型的 UV 座標正確設定
4. **檔案路徑**: 使用相對路徑，與 JSON 檔案放在同一目錄
5. **命名規範**: 使用描述性名稱，如 `wood_texture.png` 而不是 `tex1.png`

## 範例檔案 (Example Files)

專案包含以下範例檔案：

- `example_3d_scene.json` - 使用外部 OBJ 模型和紋理
- `example_3d_scene_inline.json` - 使用內嵌網格和紋理
- `cube.obj` - 範例 3D 模型（包含 UV 座標）
- `texture.png` - 範例紋理檔案（需要提供）

## 疑難排解 (Troubleshooting)

### 問題: 紋理未顯示 (Texture Not Showing)

**可能原因 (Possible Causes)**:
1. 紋理檔案路徑錯誤
2. 模型沒有 UV 座標
3. SDL_image 未正確安裝

**解決方案 (Solutions)**:
1. 檢查檔案路徑和檔案是否存在
2. 確保 OBJ 檔案包含 `vt` 指令
3. 安裝 SDL_image 函式庫

### 問題: 紋理扭曲 (Texture Distortion)

**可能原因**:
1. UV 座標映射錯誤
2. 紋理尺寸比例不正確

**解決方案**:
1. 在 3D 建模軟體中重新調整 UV 映射
2. 使用正方形紋理圖片

## 參考資料 (References)

- [Wavefront OBJ Format](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
- [UV Mapping](https://en.wikipedia.org/wiki/UV_mapping)
- [SDL_image Documentation](https://www.libsdl.org/projects/SDL_image/)
- `babylon3D.h` - 紋理結構定義
- `scene_json.h` - 場景和模型結構定義

## 授權 (License)

此實作遵循與 SDLMM 專案相同的 BSD 3-Clause License。
