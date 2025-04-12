package main

import (
	"image"
	"image/color"
	"math"
)

type YCbCr struct {
	K      int
	Filter ErrorFilter
}

func (ycbycr YCbCr) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	width, height := bounds.Dx(), bounds.Dy()

	Y := make([][]float64, height)
	Cb := make([][]float64, height)
	Cr := make([][]float64, height)

	for y := 0; y < height; y++ {
		Y[y] = make([]float64, width)
		Cb[y] = make([]float64, width)
		Cr[y] = make([]float64, width)
		for x := 0; x < width; x++ {
			r, g, b, _ := img.At(x, y).RGBA()

			r8, g8, b8 := float64(r/257), float64(g/257), float64(b/257)
			Y[y][x] = 0.299*r8 + 0.587*g8 + 0.114*b8
			Cb[y][x] = -0.168736*r8 - 0.331264*g8 + 0.5*b8 + 128
			Cr[y][x] = 0.5*r8 - 0.418688*g8 - 0.081312*b8 + 128
		}
	}

	step := 255.0 / float64(ycbycr.K-1)
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			kY := math.Round(Y[y][x]/step) * step

			errY := Y[y][x] - kY
			Y[y][x] = kY

			for j := -ycbycr.Filter.Fy; j <= ycbycr.Filter.Fy; j++ {
				for i := -ycbycr.Filter.Fx; i <= ycbycr.Filter.Fx; i++ {
					neighborX := x + i
					neighborY := y + j

					if neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height {
						weight := ycbycr.Filter.Matrix[j+ycbycr.Filter.Fy][i+ycbycr.Filter.Fx]
						Y[neighborY][neighborX] += errY * weight
					}
				}
			}
		}
	}

	newImg := image.NewRGBA(bounds)
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			yVal := Y[y][x]
			cbVal := Cb[y][x]
			crVal := Cr[y][x]

			r := yVal + 1.402*(crVal-128)
			g := yVal - 0.344136*(cbVal-128) - 0.714136*(crVal-128)
			b := yVal + 1.772*(cbVal-128)

			r = math.Max(0, math.Min(255, r))
			g = math.Max(0, math.Min(255, g))
			b = math.Max(0, math.Min(255, b))

			newImg.Set(x, y, color.RGBA{
				R: uint8(r),
				G: uint8(g),
				B: uint8(b),
				A: 255,
			})
		}
	}

	return newImg
}

var YCbCrFilter = YCbCr{K: 2, Filter: FloydSteinbergFilter}
