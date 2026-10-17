#!/usr/bin/env python3
"""
Generate cortex cross-hatch textures for archaeological illustration
Raczynski-Henk 2017 standards: 45° and -45° cross-hatching pattern
"""

from PIL import Image, ImageDraw
import os

def create_cross_hatch_texture(size=512, line_width=2, spacing=20, output_path="cortex_cross_hatch.png"):
    """
    Create a cross-hatch pattern with 45° and -45° diagonal lines

    Args:
        size: Texture size in pixels (square)
        line_width: Width of hatch lines in pixels
        spacing: Spacing between parallel lines
        output_path: Output PNG file path
    """
    # Create white background image
    img = Image.new('L', (size, size), 255)
    draw = ImageDraw.Draw(img)

    # Draw 45° hatching lines (upper-left to lower-right)
    for i in range(-size, size * 2, spacing):
        draw.line([(0, i), (i, 0)], fill=0, width=line_width)
        draw.line([(size, i + size), (i + size, size)], fill=0, width=line_width)

    # Draw -45° hatching lines (upper-right to lower-left)
    for i in range(-size, size * 2, spacing):
        draw.line([(size, i), (size - i, 0)], fill=0, width=line_width)
        draw.line([(0, i + size), (size - i - size, size)], fill=0, width=line_width)

    # Save the texture
    img.save(output_path)
    print(f"Created: {output_path} (size={size}, spacing={spacing}, line_width={line_width})")

    return img

def main():
    """Generate three density levels of cortex cross-hatch textures"""

    # Get the script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Texture parameters
    size = 512
    line_width = 2

    # Generate three density levels
    densities = [
        ("light", 20),   # Light density: 20px spacing
        ("medium", 15),  # Medium density: 15px spacing
        ("dense", 10)    # Dense: 10px spacing
    ]

    print("Generating cortex cross-hatch textures for archaeological illustration...")
    print(f"Raczynski-Henk 2017 standards: 45° and -45° cross-hatching")
    print("-" * 70)

    for density_name, spacing in densities:
        output_path = os.path.join(script_dir, f"cortex_cross_hatch_{density_name}.png")
        create_cross_hatch_texture(
            size=size,
            line_width=line_width,
            spacing=spacing,
            output_path=output_path
        )

    print("-" * 70)
    print("All textures generated successfully!")
    print(f"\nFiles created in: {script_dir}/")
    print("  - cortex_cross_hatch_light.png  (20px spacing)")
    print("  - cortex_cross_hatch_medium.png (15px spacing)")
    print("  - cortex_cross_hatch_dense.png  (10px spacing)")

if __name__ == "__main__":
    main()
