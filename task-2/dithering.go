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

	// Create separate error buffers for each channel
	SR := make([][]float64, height)
	SG := make([][]float64, height)
	SB := make([][]float64, height)

	for y := 0; y < height; y++ {
		SR[y] = make([]float64, width)
		SG[y] = make([]float64, width)
		SB[y] = make([]float64, width)
		for x := 0; x < width; x++ {
			r, g, b, _ := img.At(x, y).RGBA()
			SR[y][x] = float64(r / 257) // Normalize to 8-bit
			SG[y][x] = float64(g / 257)
			SB[y][x] = float64(b / 257)
		}
	}

	stepR := 255.0 / float64(e.KR-1)
	stepG := 255.0 / float64(e.KG-1)
	stepB := 255.0 / float64(e.KB-1)

	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			// Quantize each channel
			KR := math.Round(SR[y][x]/stepR) * stepR
			KG := math.Round(SG[y][x]/stepG) * stepG
			KB := math.Round(SB[y][x]/stepB) * stepB

			// Set the quantized pixel
			newImg.Set(x, y, color.RGBA{
				R: uint8(KR),
				G: uint8(KG),
				B: uint8(KB),
				A: 255,
			})

			// Calculate quantization errors
			errR := SR[y][x] - KR
			errG := SG[y][x] - KG
			errB := SB[y][x] - KB

			// Distribute errors to neighboring pixels
			for j := -e.Filter.Fy; j <= e.Filter.Fy; j++ {
				for i := -e.Filter.Fx; i <= e.Filter.Fx; i++ {
					neighborX := x + i
					neighborY := y + j

					if neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height {
						weight := e.Filter.Matrix[j+e.Filter.Fy][i+e.Filter.Fx]
						SR[neighborY][neighborX] += errR * weight
						SG[neighborY][neighborX] += errG * weight
						SB[neighborY][neighborX] += errB * weight
					}
				}
			}
		}
	}

	return newImg
}
