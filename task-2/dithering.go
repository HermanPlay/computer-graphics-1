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
	K      int
	Filter ErrorFilter
}

func (e ErrorDithering) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	width, height := bounds.Dx(), bounds.Dy()

	newImg := image.NewRGBA(bounds)

	S := make([][]float64, height)
	for y := 0; y < height; y++ {
		S[y] = make([]float64, width)
		for x := 0; x < width; x++ {
			r, g, b, _ := img.At(x, y).RGBA()
			S[y][x] = float64(PixelToGray(r, g, b))
		}
	}

	step := 255.0 / float64(e.K-1)

	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			K := math.Round(S[y][x]/step) * step
			newImg.Set(x, y, color.Gray{Y: uint8(K)})

			err := S[y][x] - K

			for j := -e.Filter.Fy; j <= e.Filter.Fy; j++ {
				for i := -e.Filter.Fx; i <= e.Filter.Fx; i++ {
					neighborX := x + i
					neighborY := y + j

					if neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height {
						S[neighborY][neighborX] += err * e.Filter.Matrix[j+e.Filter.Fy][i+e.Filter.Fx]
					}
				}
			}
		}
	}

	return newImg
}
