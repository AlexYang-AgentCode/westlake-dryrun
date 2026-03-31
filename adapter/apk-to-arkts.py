#!/usr/bin/env python3
"""
APK → ArkTS Adapter Generator

Reads an Android APK, extracts manifest + layout XML (via aapt2 dump),
and generates an equivalent OpenHarmony ArkTS project that displays
the same UI.

Usage: python3 apk-to-arkts.py <apk_path> <output_dir>
"""

import subprocess
import sys
import os
import re
import json

AAPT2 = "/mnt/e/10.Project/16-WestLake/16.22-HelloWorld/android-sdk/build-tools/35.0.0/aapt2"


def dump_manifest(apk_path: str) -> dict:
    """Extract package name and launcher activity from manifest."""
    result = subprocess.run(
        [AAPT2, "dump", "xmltree", apk_path, "--file", "AndroidManifest.xml"],
        capture_output=True, text=True
    )
    text = result.stdout

    pkg = ""
    activities = []
    current_activity = None
    is_launcher = False

    for line in text.split("\n"):
        if "package=" in line:
            m = re.search(r'package="([^"]+)"', line)
            if m:
                pkg = m.group(1)
        if "E: activity" in line:
            current_activity = {}
        if current_activity is not None:
            if ":name(" in line:
                m = re.search(r'"([^"]+)"', line)
                if m:
                    name = m.group(1)
                    if name.startswith("."):
                        name = pkg + name
                    current_activity["name"] = name
            if "action.MAIN" in line:
                is_launcher = True
            if "category.LAUNCHER" in line and is_launcher:
                current_activity["launcher"] = True

        if "E: activity" in line and current_activity and "name" in current_activity:
            activities.append(current_activity)
            current_activity = {}

    # Finalize last activity
    if current_activity and "name" in current_activity:
        activities.append(current_activity)

    launcher = next((a for a in activities if a.get("launcher")), activities[0] if activities else {"name": "MainActivity"})

    return {"package": pkg, "launcher": launcher["name"], "activities": activities}


def dump_layout(apk_path: str, layout_name: str = "res/layout/activity_main.xml") -> list:
    """Parse aapt2 xmltree output into a view tree."""
    result = subprocess.run(
        [AAPT2, "dump", "xmltree", apk_path, "--file", layout_name],
        capture_output=True, text=True
    )

    nodes = []
    stack = []

    for line in result.stdout.split("\n"):
        # Count indentation to determine nesting level
        stripped = line.lstrip()
        indent = len(line) - len(stripped)
        level = indent // 2

        if stripped.startswith("E: "):
            tag = stripped.split()[1]
            node = {"tag": tag, "attrs": {}, "children": [], "level": level}

            # Pop stack to find parent
            while stack and stack[-1]["level"] >= level:
                stack.pop()

            if stack:
                stack[-1]["children"].append(node)
            else:
                nodes.append(node)

            stack.append(node)

        elif stripped.startswith("A: ") and stack:
            attr_line = stripped[3:]
            # Parse attribute name and value
            # Format: http://schemas.android.com/apk/res/android:name(0x...)=value
            m = re.match(r'(?:http://[^:]+:)?(\w+)\([^)]+\)=(.+)', attr_line)
            if m:
                attr_name = m.group(1)
                attr_value = m.group(2).strip()
                # Clean up value - extract quoted strings
                if '"' in attr_value:
                    qm = re.search(r'"([^"]*)"', attr_value)
                    if qm:
                        attr_value = qm.group(1)
                stack[-1]["attrs"][attr_name] = attr_value

    return nodes


def android_attr_to_arkui(attr_name: str, attr_value: str) -> str:
    """Convert a single Android XML attribute to ArkUI chained method."""
    mapping = {
        "text": lambda v: f"// text set in constructor",
        "textSize": lambda v: f".fontSize({float(re.sub(r'[a-z]+', '', v))})",
        "textColor": lambda v: f".fontColor('{v.replace('ff', '#', 1) if v.startswith('#ff') else v}')" if v.startswith("#") else "",
        "textStyle": lambda v: ".fontWeight(FontWeight.Bold)" if v in ("0x00000001", "1", "bold") else "",
        "layout_marginTop": lambda v: f".margin({{ top: {float(re.sub(r'[a-z]+', '', v))} }})",
        "background": lambda v: f".backgroundColor('{v}')" if v.startswith("#") else "",
    }

    if attr_name in mapping:
        return mapping[attr_name](attr_value)
    return ""


def view_to_arkui(node: dict, indent: int = 6) -> str:
    """Convert an Android View tree node to ArkUI build() code."""
    tag = node["tag"]
    attrs = node["attrs"]
    children = node["children"]
    pad = " " * indent

    lines = []

    if tag == "LinearLayout":
        orientation = attrs.get("orientation", "1")
        container = "Column" if orientation == "1" else "Row"
        lines.append(f"{pad}{container}() {{")

        for child in children:
            lines.append(view_to_arkui(child, indent + 2))

        lines.append(f"{pad}}}")

        # Container attributes
        if attrs.get("layout_width") == "-1":
            lines.append(f"{pad}.width('100%')")
        if attrs.get("layout_height") == "-1":
            lines.append(f"{pad}.height('100%')")

        gravity = attrs.get("gravity", "")
        if gravity in ("0x00000011", "center"):
            lines.append(f"{pad}.justifyContent(FlexAlign.Center)")
            lines.append(f"{pad}.alignItems(HorizontalAlign.Center)")

        bg = attrs.get("background", "")
        if bg.startswith("#"):
            lines.append(f"{pad}.backgroundColor('{bg}')")

    elif tag == "TextView":
        text = attrs.get("text", "")
        lines.append(f"{pad}Text('{text}')")

        # Apply text attributes
        text_size = attrs.get("textSize", "")
        if text_size:
            size = float(re.sub(r'[a-z]+', '', text_size))
            lines.append(f"{pad}  .fontSize({size})")

        text_style = attrs.get("textStyle", "")
        if text_style in ("0x00000001", "1", "bold"):
            lines.append(f"{pad}  .fontWeight(FontWeight.Bold)")

        text_color = attrs.get("textColor", "")
        if text_color.startswith("#"):
            # Convert #ffXXXXXX to #XXXXXX
            color = text_color
            if len(color) == 10:  # #ffRRGGBB
                color = "#" + color[4:]
            lines.append(f"{pad}  .fontColor('{color}')")

        margin_top = attrs.get("layout_marginTop", "")
        if margin_top:
            val = float(re.sub(r'[a-z]+', '', margin_top))
            lines.append(f"{pad}  .margin({{ top: {val} }})")

    elif tag == "Button":
        text = attrs.get("text", "Button")
        lines.append(f"{pad}Button('{text}')")

    else:
        # Unknown view - render as comment + Column
        lines.append(f"{pad}// TODO: unsupported view '{tag}'")
        lines.append(f"{pad}Column() {{")
        for child in children:
            lines.append(view_to_arkui(child, indent + 2))
        lines.append(f"{pad}}}")

    return "\n".join(lines)


def generate_page(view_tree: list, page_name: str = "AdaptedPage") -> str:
    """Generate a complete ArkTS page from the view tree."""
    body = "\n".join(view_to_arkui(node) for node in view_tree)

    return f"""@Entry
@Component
struct {page_name} {{
  build() {{
{body}
  }}
}}
"""


def generate_ability(manifest: dict) -> str:
    """Generate EntryAbility.ets that bridges Activity lifecycle."""
    return f"""import {{ UIAbility, AbilityConstant, Want }} from '@kit.AbilityKit';
import {{ hilog }} from '@kit.PerformanceAnalysisKit';
import {{ window }} from '@kit.ArkUI';

const TAG = 'Adapter';
const DOMAIN = 0x0001;

/**
 * AdapterAbility — bridges Android Activity lifecycle to OH UIAbility.
 *
 * Original Android app: {manifest['package']}
 * Launcher Activity: {manifest['launcher']}
 *
 * Lifecycle mapping:
 *   Activity.onCreate()  → UIAbility.onCreate() + onWindowStageCreate()
 *   Activity.onResume()  → UIAbility.onForeground()
 *   Activity.onPause()   → UIAbility.onBackground()
 *   Activity.onDestroy() → UIAbility.onDestroy()
 *   setContentView()     → windowStage.loadContent()
 */
export default class AdapterAbility extends UIAbility {{
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {{
    hilog.info(DOMAIN, TAG, 'onCreate — bridging {manifest["launcher"]}');
  }}

  onWindowStageCreate(windowStage: window.WindowStage): void {{
    hilog.info(DOMAIN, TAG, 'onWindowStageCreate — equivalent to setContentView()');
    // This replaces: setContentView(R.layout.activity_main)
    windowStage.loadContent('pages/AdaptedPage', (err) => {{
      if (err.code) {{
        hilog.error(DOMAIN, TAG, 'loadContent failed: %{{public}}s', JSON.stringify(err));
      }}
    }});
  }}

  onForeground(): void {{
    hilog.info(DOMAIN, TAG, 'onForeground — equivalent to Activity.onResume()');
  }}

  onBackground(): void {{
    hilog.info(DOMAIN, TAG, 'onBackground — equivalent to Activity.onPause()');
  }}

  onDestroy(): void {{
    hilog.info(DOMAIN, TAG, 'onDestroy');
  }}
}}
"""


def generate_project(apk_path: str, output_dir: str):
    """Generate a complete OH project from an APK."""
    print(f"Parsing APK: {apk_path}")

    manifest = dump_manifest(apk_path)
    print(f"  Package: {manifest['package']}")
    print(f"  Launcher: {manifest['launcher']}")

    view_tree = dump_layout(apk_path)
    print(f"  Layout nodes: {len(view_tree)}")

    # Generate ArkTS code
    page_code = generate_page(view_tree)
    ability_code = generate_ability(manifest)

    # Write to output
    ets_dir = os.path.join(output_dir, "entry", "src", "main", "ets")
    os.makedirs(os.path.join(ets_dir, "entryability"), exist_ok=True)
    os.makedirs(os.path.join(ets_dir, "pages"), exist_ok=True)

    with open(os.path.join(ets_dir, "entryability", "EntryAbility.ets"), "w") as f:
        f.write(ability_code)
    print(f"  Generated: EntryAbility.ets")

    with open(os.path.join(ets_dir, "pages", "AdaptedPage.ets"), "w") as f:
        f.write(page_code)
    print(f"  Generated: AdaptedPage.ets")

    # Generate module config that points to AdaptedPage
    pages_json = {"src": ["pages/AdaptedPage"]}
    res_dir = os.path.join(output_dir, "entry", "src", "main", "resources", "base", "profile")
    os.makedirs(res_dir, exist_ok=True)
    with open(os.path.join(res_dir, "main_pages.json"), "w") as f:
        json.dump(pages_json, f)

    print(f"\nDone! Generated OH project in: {output_dir}")
    print(f"  Ability: {ets_dir}/entryability/EntryAbility.ets")
    print(f"  Page:    {ets_dir}/pages/AdaptedPage.ets")
    print(f"\nThe adapted page should display the same UI as the original APK.")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <apk_path> <output_dir>")
        sys.exit(1)

    generate_project(sys.argv[1], sys.argv[2])
