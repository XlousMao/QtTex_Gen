

## 项目结构
```text
QtTex_Gen/
│
├── core/                   # 核心逻辑层 (完全独立于 UI)
│   ├── AnkiCard.h/cpp      # 数据实体类结构
│   ├── CardParser.h/cpp    # MD 文本解析与切割器
│   ├── LatexConverter.h/cpp# 公式正则转换工具箱
│   └── FileHandler.h/cpp   # 文件读写封装
│
├── ui/                     # 用户界面层 (View)
│   ├── MainWindow.h/cpp    # 主窗口逻辑
│   └── MainWindow.ui       # Qt Designer 界面设计文件
│
├── resources/              # 资源文件
│
└── main.cpp                # 程序入口
```