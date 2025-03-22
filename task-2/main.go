package main

import (
	"fmt"
	"image"
	"image/color"
	"image/png"
	"io"
	"log"
	"os"
	"strconv"

	"gioui.org/app"
	"gioui.org/io/event"
	"gioui.org/layout"
	"gioui.org/op"
	"gioui.org/op/clip"
	"gioui.org/op/paint"
	"gioui.org/unit"
	"gioui.org/widget"
	"gioui.org/widget/material"
	"gioui.org/x/explorer"
)

const className = "gio"
const brighthnessFactor = 1.2
const contrastFactor = 0.2
const gammaValue = 1.5

type ImageResult struct {
	Error  error
	Format string
	Image  image.Image
}

func drawSeparator(gtx layout.Context) layout.Dimensions {
	bounds := image.Rect(0, 5, gtx.Constraints.Max.X-5, 5)

	paint.FillShape(gtx.Ops, color.NRGBA{0, 0, 0, 255}, clip.Stroke{Path: clip.Rect(bounds).Path(), Width: 2}.Op())
	d := image.Point{Y: 10}
	return layout.Dimensions{Size: d}
}

func SaveImage(img image.Image, writer io.Writer) error {
	return png.Encode(writer, img)
}

func main() {
	go func() {
		window := new(app.Window)
		window.Option(app.Title(className))
		window.Option(app.MinSize(1920, 1080))

		err := run(window)
		if err != nil {
			log.Fatal(err)
		}
		os.Exit(0)
	}()
	app.Main()
}

func run(window *app.Window) error {
	theme := material.NewTheme()
	var ops op.Ops
	expl := explorer.NewExplorer(window)
	imgChan := make(chan ImageResult)
	events := make(chan event.Event)
	acks := make(chan struct{})
	var filteredImage *FilteredImage
	openButton := new(widget.Clickable)
	saveButton := new(widget.Clickable)
	inversionButton := new(widget.Clickable)
	brightnessButton := new(widget.Clickable)
	contrastButton := new(widget.Clickable)
	gammaButton := new(widget.Clickable)
	resetButton := new(widget.Clickable)
	loadKernelButton := new(widget.Clickable)
	saveKernelButton := new(widget.Clickable)
	applyKernelButton := new(widget.Clickable)
	erosionButton := new(widget.Clickable)
	dilationButton := new(widget.Clickable)
	grayScaleButton := new(widget.Clickable)
	errorDitheringButton := new(widget.Clickable)
	uniformQuantizationButton := new(widget.Clickable)
	convolutionEditor := NewConvolutionEditor()
	var errorMessage *string = new(string)
	originalImageScroll := widget.List{List: layout.List{Axis: layout.Horizontal}}
	filteredImageScroll := widget.List{List: layout.List{Axis: layout.Vertical}}
	kInput := new(widget.Editor)
	kInput.SetText("2") // Default value for k

	filterSelect := new(widget.Enum)
	filterSelect.Value = "FloydSteinberg" // Default filter

	filterOptions := map[string]ErrorFilter{
		"FloydSteinberg": FloydSteinbergFilter,
		"Burkes":         BurkesFilter,
		"Stucki":         StuckiFilter,
		"Sierra":         SierraFilter,
		"Atkinson":       AtkinsonFilter,
	}

	go func() {
		for {
			ev := window.Event()
			events <- ev
			<-acks
			if _, ok := ev.(app.DestroyEvent); ok {
				return
			}
		}
	}()
	for {
		select {
		case imgResult := <-imgChan:
			filteredImage = NewFilteredImage(imgResult.Image)
			window.Invalidate()
		case e := <-events:
			switch e := e.(type) {
			case app.DestroyEvent:
				return e.Err
			case app.FrameEvent:
				gtx := app.NewContext(&ops, e)

				if openButton.Clicked(gtx) {
					go func() {
						file, err := expl.ChooseFile("png", "jpeg", "jpg")
						if err != nil {
							fmt.Printf("failed opening image file: %v", err)
							return
						}
						defer file.Close()
						imgData, format, err := image.Decode(file)
						if err != nil {
							fmt.Printf("failed decoding image data: %v", err)
							return
						}
						*errorMessage = ""
						imgChan <- ImageResult{Image: imgData, Format: format}
					}()
				}
				if saveButton.Clicked(gtx) {
					go func() {
						file, err := expl.CreateFile("image.png")
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel file: %v", err)
							return
						}
						defer file.Close()
						err = SaveImage(filteredImage.FilteredImage, file)
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save image: %v", err)
							return
						}
						*errorMessage = ""
					}()
				}
				if inversionButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.AddFilter(InversionFilter{})
						window.Invalidate()
					}
				}
				if brightnessButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.AddFilter(BrightnessFilter{Factor: brighthnessFactor})
						window.Invalidate()
					}
				}
				if contrastButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.AddFilter(ContrastFilter{Factor: contrastFactor})
						window.Invalidate()
					}
				}
				if gammaButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.AddFilter(GammaFilter{Gamma: gammaValue})
						window.Invalidate()
					}
				}
				if erosionButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.AddFilter(MorphologicalFilter{Size: 3, IsErosion: true})
						window.Invalidate()
					}
				}
				if dilationButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.AddFilter(MorphologicalFilter{Size: 3, IsErosion: false})
						window.Invalidate()
					}
				}
				if grayScaleButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.AddFilter(GrayscaleFilter{})
						window.Invalidate()
					}
				}
				if errorDitheringButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						k, err := strconv.Atoi(kInput.Text())
						if err != nil || k < 2 {
							*errorMessage = "Invalid value for k. Must be an integer >= 2."
						} else {
							selectedFilter, ok := filterOptions[filterSelect.Value]
							if !ok {
								*errorMessage = "Invalid filter selected."
							} else {
								filteredImage.AddFilter(ErrorDithering{
									K:      k,
									Filter: selectedFilter,
								})
								window.Invalidate()
							}
						}
					}
				}
				if uniformQuantizationButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						k, err := strconv.Atoi(kInput.Text())
						if err != nil || k < 2 {
							*errorMessage = "Invalid value for k. Must be an integer >= 2."
						} else {
							uk := uint32(k)
							filteredImage.AddFilter(UniformQuantization{
								KR: uk,
								KG: uk,
								KB: uk,
							})
							window.Invalidate()
						}
					}
				}
				if resetButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.ResetFilters()
						window.Invalidate()
					}
				}
				if loadKernelButton.Clicked(gtx) {
					go func() {
						file, err := expl.ChooseFile("filter")
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to open kernel file: %v", err)
							return
						}
						defer file.Close()
						filter, err := LoadConvolutionFilter(file)
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to load kernel: %v", err)
							return
						}
						convolutionEditor.LoadFilter(filter)
						*errorMessage = ""
					}()
				}
				if saveKernelButton.Clicked(gtx) {
					go func() {
						file, err := expl.CreateFile("filter.filter")
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel file: %v", err)
							return
						}
						defer file.Close()
						filter, err := convolutionEditor.GetFilter()
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel: %v", err)
							return
						}
						err = SaveConvolutionFilter(*filter, file)
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel: %v", err)
							return
						}
						*errorMessage = ""
					}()
				}
				if applyKernelButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						err := convolutionEditor.UpdateKernel()
						if err != "" {
							*errorMessage = err
						} else {
							if convolutionEditor.ErrorMessage != "" {
								*errorMessage = convolutionEditor.ErrorMessage
							} else {
								*errorMessage = ""
								log.Printf("Applying kernel: %v", convolutionEditor.Kernel)
								filteredImage.AddFilter(ConvolutionFilter{Kernel: convolutionEditor.Kernel, Divisor: convolutionEditor.Divisor, Offset: convolutionEditor.Offset, AnchorX: convolutionEditor.AnchorX, AnchorY: convolutionEditor.AnchorY})
							}
							window.Invalidate()
						}
					}
				}

				layout.Flex{
					Axis:      layout.Horizontal,
					Spacing:   layout.SpaceEvenly,
					Alignment: layout.Middle,
				}.Layout(gtx,
					layout.Flexed(7, func(gtx layout.Context) layout.Dimensions {
						return layout.Flex{
							Axis:    layout.Horizontal,
							Spacing: layout.SpaceEvenly,
						}.Layout(gtx,
							layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
								return layout.Inset{Top: unit.Dp(10), Bottom: unit.Dp(10), Left: unit.Dp(10), Right: unit.Dp(10)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
									return material.List(theme, &originalImageScroll).Layout(gtx, 1, func(gtx layout.Context, index int) layout.Dimensions {
										return material.List(theme, &filteredImageScroll).Layout(gtx, 1, func(gtx layout.Context, index int) layout.Dimensions {
											if filteredImage == nil || filteredImage.OriginalImage == nil {
												return layout.Dimensions{}
											} else {
												img := widget.Image{
													Src: paint.NewImageOp(filteredImage.OriginalImage),
													Fit: widget.Unscaled,
												}
												return img.Layout(gtx)
											}
										})
									})
								})
							}),
							layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
								return layout.Inset{Top: unit.Dp(10), Bottom: unit.Dp(10), Left: unit.Dp(10), Right: unit.Dp(10)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
									return material.List(theme, &originalImageScroll).Layout(gtx, 1, func(gtx layout.Context, index int) layout.Dimensions {
										return material.List(theme, &filteredImageScroll).Layout(gtx, 1, func(gtx layout.Context, index int) layout.Dimensions {
											if filteredImage == nil || filteredImage.OriginalImage == nil {
												return layout.Dimensions{}
											} else {
												img := widget.Image{
													Src: paint.NewImageOp(filteredImage.FilteredImage),
													Fit: widget.Unscaled,
												}
												return img.Layout(gtx)
											}
										})
									})
								})
							}),
						)
					}),
					layout.Flexed(1, func(gtx layout.Context) layout.Dimensions {
						return layout.Flex{
							Axis:    layout.Vertical,
							Spacing: layout.SpaceEnd,
						}.Layout(gtx,
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									if *errorMessage != "" {
										return layout.Inset{Top: unit.Dp(10), Bottom: unit.Dp(10), Left: unit.Dp(10), Right: unit.Dp(10)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
											caption := material.Caption(theme, *errorMessage)
											caption.Color = color.NRGBA{255, 0, 0, 255}
											return caption.Layout(gtx)
										})
									} else {
										return layout.Dimensions{}
									}
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, openButton, "Open").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, saveButton, "Save").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(drawSeparator),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, inversionButton, "Invert").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, brightnessButton, "Brighten").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, contrastButton, "Contrast").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, gammaButton, "Gamma").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, erosionButton, "Erosion").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, dilationButton, "Dilation").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, resetButton, "Reset").Layout(gtx)
									})
								},
							),
							layout.Rigid(drawSeparator),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, loadKernelButton, "Load Kernel").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, saveKernelButton, "Save Kernel").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, applyKernelButton, "Apply Kernel").Layout(gtx)
									})
								},
							),
							layout.Rigid(func(gtx layout.Context) layout.Dimensions {
								return convolutionEditor.Layout(gtx, theme)
							}),
							layout.Rigid(drawSeparator),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, grayScaleButton, "Grayscale").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceBetween,
									}.Layout(gtx,
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Body1(theme, "K:").Layout(gtx)
											})
										}),
										layout.Flexed(1, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Editor(theme, kInput, "Number of Colors (k)").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis: layout.Vertical,
									}.Layout(gtx,
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.Body1(theme, "Select Filter:").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "FloydSteinberg", "Floyd-Steinberg").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Burkes", "Burkes").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Stucki", "Stucki").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Sierra", "Sierra").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Atkinson", "Atkinson").Layout(gtx)
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, errorDitheringButton, "Error Dithering").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5), Bottom: unit.Dp(5), Left: unit.Dp(5), Right: unit.Dp(5)}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, uniformQuantizationButton, "Uniform Quantization").Layout(gtx)
									})
								},
							),
						)
					}),
				)

				e.Frame(gtx.Ops)
			}
			acks <- struct{}{}
		}
	}
}
