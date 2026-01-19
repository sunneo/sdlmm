# 光源處理說明 (Light Source Processing Documentation)

## 概述 (Overview)

JSON 場景格式完整支援 3D 光源定義。光源資訊在 JSON 中定義，載入場景時會解析並在渲染過程中使用，影響物體表面的光照效果。

The JSON scene format fully supports 3D light source definitions. Light information is defined in JSON, parsed during scene loading, and used during rendering to affect surface lighting.

## JSON 格式 (JSON Format)

### 完整場景範例 (Complete Scene Example)

```json
{
  "format": 1,
  "width": 800,
  "height": 600,
  "backgroundColor": 0,
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

### 多光源範例 (Multiple Lights Example)

```json
{
  "lights": [
    {
      "position": [10, 10, 10],
      "intensity": 1.0,
      "color": 16777215
    },
    {
      "position": [-10, 5, 0],
      "intensity": 0.5,
      "color": 16711680
    },
    {
      "position": [0, -10, 5],
      "intensity": 0.8,
      "color": 255
    }
  ]
}
```

**注意**: 目前渲染器使用第一個光源。未來版本將支援多光源渲染。

**Note**: Currently the renderer uses the first light source. Future versions will support multiple light rendering.

## 光源屬性 (Light Properties)

### position (位置)

- **類型 (Type)**: 陣列 [x, y, z]
- **必需 (Required)**: 是 (Yes)
- **描述 (Description)**: 光源在 3D 空間中的位置
- **範例 (Example)**: `[5, 5, 5]`

### intensity (強度)

- **類型 (Type)**: 浮點數 (float)
- **必需 (Required)**: 否 (No) - 預設 1.0
- **範圍 (Range)**: 0.0 到 1.0 或更高
- **描述 (Description)**: 光照強度，影響表面亮度
- **範例 (Example)**: `1.0` (全強度), `0.5` (半強度)

### color (顏色)

- **類型 (Type)**: 整數 (integer) - 0xRRGGBB 格式
- **必需 (Required)**: 否 (No) - 預設 0xFFFFFF (白色)
- **描述 (Description)**: 光源顏色
- **範例 (Examples)**:
  - `16777215` (0xFFFFFF) - 白光
  - `16711680` (0xFF0000) - 紅光
  - `65280` (0x00FF00) - 綠光
  - `255` (0x0000FF) - 藍光

## 處理流程 (Processing Flow)

### 1. JSON 解析 (JSON Parsing)

在 `scene3d_load_from_json()` 函數中:

```c
// Load lights
cJSON* lights = cJSON_GetObjectItem(root, "lights");
if (lights && cJSON_IsArray(lights)) {
    cJSON* lightItem = NULL;
    cJSON_ArrayForEach(lightItem, lights) {
        Light3D light = {0};
        
        // 解析位置
        cJSON* position = cJSON_GetObjectItem(lightItem, "position");
        light.position.x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 0));
        light.position.y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 1));
        light.position.z = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 2));
        
        // 解析強度和顏色
        light.intensity = intensity ? (float)cJSON_GetNumberValue(intensity) : 1.0f;
        light.color = color ? (int)cJSON_GetNumberValue(color) : 0xFFFFFF;
        
        scene3d_add_light(scene, &light);
    }
}
```

### 2. 儲存到場景結構 (Storage in Scene Structure)

```c
typedef struct {
    SceneFormat format;
    Camera camera;
    int lightCount;
    Light3D* lights;    // 光源陣列
    int modelCount;
    Model3D* models;
    int width;
    int height;
    int backgroundColor;
} Scene3D;
```

### 3. 傳遞給渲染器 (Passing to Renderer)

在 `scene3d_render()` 函數中:

```c
// 使用場景中的第一個光源
const Vector3* lightPos = NULL;
if (scene->lightCount > 0) {
    lightPos = &scene->lights[0].position;
}

// 傳遞給渲染函數
device_render(device, &scene->camera, meshArray, scene->modelCount, lightPos);
```

### 4. 光照計算 (Lighting Calculation)

在 `device_drawTriangle()` 中使用光源位置:

```c
// 為每個頂點計算光照 (N·L)
float nl1 = device_computeNDotL(&v1->WorldCoordinates, &v1->Normal, lightPos);
float nl2 = device_computeNDotL(&v2->WorldCoordinates, &v2->Normal, lightPos);
float nl3 = device_computeNDotL(&v3->WorldCoordinates, &v3->Normal, lightPos);
```

N·L (法線點光源方向) 計算公式:

```c
float device_computeNDotL(const Vector3* vertex, Vector3* normal, Vector3* lightPosition) {
    Vector3 lightDirection = vector3_subtract(lightPosition, vertex);
    vector3_normalize(normal);
    vector3_normalize(&lightDirection);
    return maxf(0, vector3_dot(normal, &lightDirection));
}
```

## 光照模型 (Lighting Model)

### Lambertian 反射 (Lambertian Reflection)

當前使用的是簡單的 Lambertian (漫反射) 光照模型:

**公式**: `Light = ambient + diffuse * (N · L)`

其中:
- **ambient**: 環境光 (0.2)
- **diffuse**: 漫反射係數 (0.8)
- **N**: 表面法線向量
- **L**: 光源方向向量
- **(N · L)**: 法線和光源方向的點積

在 `device_processScanLine()` 中應用:

```c
float lightingFactor = 0.2f + 0.8f * ndotl;
device_drawPoint(dev, &pt, device_color4ref(textureColor, 
    lightingFactor, lightingFactor, lightingFactor, 1));
```

## 使用範例 (Usage Examples)

### 範例 1: 頂部光源 (Top Light)

```json
{
  "lights": [
    {
      "position": [0, 10, 0],
      "intensity": 1.0,
      "color": 16777215
    }
  ]
}
```

效果: 光從上方照射，物體頂部較亮，底部較暗

### 範例 2: 側面彩色光 (Side Colored Light)

```json
{
  "lights": [
    {
      "position": [10, 5, 5],
      "intensity": 0.8,
      "color": 16711680
    }
  ]
}
```

效果: 紅色光從右側照射

### 範例 3: 無光源 (No Light)

```json
{
  "lights": []
}
```

效果: 使用預設光源位置 (0, 10, 10)

## 預設光源 (Default Light)

如果 JSON 中沒有定義光源，系統使用預設光源:

```c
Vector3 defaultLight = vector3(0, 10, 10);
```

- 位置: (0, 10, 10)
- 強度: 1.0
- 顏色: 白色

## 程式化使用 (Programmatic Usage)

### 創建光源

```c
#include "scene_json.h"

// 創建場景
Scene3D* scene = scene3d_create(800, 600);

// 添加光源
Light3D light = {0};
light.position = vector3(5.0f, 5.0f, 5.0f);
light.intensity = 1.0f;
light.color = 0xFFFFFF;
scene3d_add_light(scene, &light);

// 保存 (會包含光源資訊)
scene3d_save_to_json(scene, "scene.json");
```

### 載入和使用光源

```c
// 載入場景 (自動載入光源)
Scene3D* scene = scene3d_load_from_json("scene.json");

printf("光源數量: %d\n", scene->lightCount);
for (int i = 0; i < scene->lightCount; i++) {
    printf("光源 %d 位置: (%.2f, %.2f, %.2f)\n", i,
           scene->lights[i].position.x,
           scene->lights[i].position.y,
           scene->lights[i].position.z);
}

// 渲染 (自動使用場景光源)
Device* device = device(800, 600);
scene3d_render(scene, device);
```

## 光源位置建議 (Light Position Recommendations)

### 主光源 (Key Light)

```json
{"position": [5, 5, 5], "intensity": 1.0}
```

從右上方照射，產生主要光照和陰影

### 補光 (Fill Light)

```json
{"position": [-3, 2, 5], "intensity": 0.5}
```

從左側照射，減少陰影過深

### 背光 (Back Light)

```json
{"position": [0, 3, -5], "intensity": 0.7}
```

從後方照射，增加輪廓感

## 限制與未來改進 (Limitations and Future Improvements)

### 當前限制 (Current Limitations)

1. **單一光源渲染**: 目前只使用第一個光源
2. **無光源衰減**: 光照強度不隨距離衰減
3. **無陰影**: 不計算物體投射的陰影
4. **無鏡面反射**: 只有漫反射，沒有高光效果

### 計劃改進 (Planned Improvements)

1. **多光源支援**: 累加所有光源的貢獻
2. **光源衰減**: 實現距離平方反比衰減
3. **點光源/方向光**: 支援不同類型的光源
4. **陰影映射**: 基本陰影計算
5. **鏡面反射**: Phong 或 Blinn-Phong 光照模型

## 技術細節 (Technical Details)

### Light3D 結構

```c
typedef struct {
    Vector3 position;    // 3D 位置
    float intensity;     // 強度 (0.0 - 1.0+)
    int color;          // 顏色 (0xRRGGBB)
} Light3D;
```

### API 函數

```c
// 添加光源到場景
void scene3d_add_light(Scene3D* scene, const Light3D* light);

// 渲染 (使用場景光源)
void scene3d_render(const Scene3D* scene, Device* device);

// 低層渲染 (直接指定光源)
void device_render(Device* dev, const Camera* camera, 
                   const Mesh* meshes, int meshesLength, 
                   const Vector3* lightPosition);
```

## 疑難排解 (Troubleshooting)

### 問題: 物體太暗 (Objects Too Dark)

**原因**: 光源位置不合適或強度太低

**解決方案**:
1. 調整光源位置更靠近物體
2. 增加 intensity 值 (可以 > 1.0)
3. 使用白色光源 (0xFFFFFF)

### 問題: 光照不均勻 (Uneven Lighting)

**原因**: 模型法線不正確

**解決方案**:
1. 檢查 OBJ 檔案的法線定義
2. 在 3D 建模軟體中重新計算法線
3. 確保法線方向朝外

### 問題: 物體全白 (Objects All White)

**原因**: 光源太強或位置太近

**解決方案**:
1. 降低 intensity 值
2. 移動光源更遠一些
3. 檢查環境光設定

## 參考資料 (References)

- [Phong Reflection Model](https://en.wikipedia.org/wiki/Phong_reflection_model)
- [Lambertian Reflectance](https://en.wikipedia.org/wiki/Lambertian_reflectance)
- `scene_json.h` - Light3D 結構定義
- `babylon3D.c` - 光照計算實作
- `example_3d_scene.json` - 光源使用範例

## 授權 (License)

此實作遵循與 SDLMM 專案相同的 BSD 3-Clause License。
