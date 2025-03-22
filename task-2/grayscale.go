package main

import (
	"image"
	"image/color"
)

// GrayscaleFilter implements the Filter interface for converting an image to grayscale.
type GrayscaleFilter struct{}

func (f GrayscaleFilter) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	grayImg := image.NewGray(bounds)

	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			r, g, b, _ := img.At(x, y).RGBA()
			grayValue := PixelToGray(r, g, b)
			grayImg.Set(x, y, color.Gray{Y: grayValue})
		}
	}

	return grayImg
}

func PixelToGray(r, g, b uint32) uint8 {
	return uint8((0.299*float64(r) + 0.587*float64(g) + 0.114*float64(b)) / 256)
}
