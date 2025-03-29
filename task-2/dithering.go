package main

import (
	"image"
	"image/color"
	"math"
)

// ErrorFilter represents the error distribution filter with its dimensions.
type ErrorFilter struct {
	Matrix [][]float64
	Fx     int
	Fy     int
}

type ErrorDithering struct {
	KR     uint32
	KG     uint32
	KB     uint32
	Filter ErrorFilter
}

func (e ErrorDithering) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	width, height := bounds.Dx(), bounds.Dy()

	newImg := image.NewRGBA(bounds)

	sR := make([][]float64, height)
	sG := make([][]float64, height)
	sB := make([][]float64, height)

	for y := range height {
		sR[y] = make([]float64, width)
		sG[y] = make([]float64, width)
		sB[y] = make([]float64, width)
		for x := range width {
			r, g, b, _ := img.At(x, y).RGBA()
			sR[y][x] = float64(r / 257)
			sG[y][x] = float64(g / 257)
			sB[y][x] = float64(b / 257)
		}
	}

	stepR := 255.0 / float64(e.KR-1)
	stepG := 255.0 / float64(e.KG-1)
	stepB := 255.0 / float64(e.KB-1)

	for y := range height {
		for x := range width {
			// Quantize each channel
			kR := math.Round(sR[y][x]/stepR) * stepR
			kG := math.Round(sG[y][x]/stepG) * stepG
			kB := math.Round(sB[y][x]/stepB) * stepB

			// Set the quantized pixel
			newImg.Set(x, y, color.RGBA{
				R: uint8(kR),
				G: uint8(kG),
				B: uint8(kB),
				A: 255,
			})

			// Calculate quantization errors
			errR := sR[y][x] - kR
			errG := sG[y][x] - kG
			errB := sB[y][x] - kB

			// Distribute errors to neighboring pixels
			for j := -e.Filter.Fy; j <= e.Filter.Fy; j++ {
				for i := -e.Filter.Fx; i <= e.Filter.Fx; i++ {
					neighborX := x + i
					neighborY := y + j

					if neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height {
						weight := e.Filter.Matrix[j+e.Filter.Fy][i+e.Filter.Fx]
						sR[neighborY][neighborX] += errR * weight
						sG[neighborY][neighborX] += errG * weight
						sB[neighborY][neighborX] += errB * weight
					}
				}
			}
		}
	}

	return newImg
}
