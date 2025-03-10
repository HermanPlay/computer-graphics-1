package main

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	"gioui.org/layout"
	"gioui.org/unit"
	"gioui.org/widget"
	"gioui.org/widget/material"
)

type ConvolutionEditor struct {
	Kernel       [][]float64
	Divisor      float64
	Offset       float64
	AnchorX      int
	AnchorY      int
	AutoDivisor  widget.Bool
	KernelField  widget.Editor
	DivisorField widget.Editor
	OffsetField  widget.Editor
	AnchorXField widget.Editor
	AnchorYField widget.Editor
	ErrorLabel   widget.Label
	ErrorMessage string
}

func NewConvolutionEditor() *ConvolutionEditor {
	editor := &ConvolutionEditor{
		Divisor: 1.0,
		Offset:  0.0,
		AnchorX: 1,
		AnchorY: 1,
	}
	editor.AutoDivisor.Value = true
	editor.KernelField.SetText("1 1 1\n1 1 1\n1 1 1")
	return editor
}

func (ce *ConvolutionEditor) Layout(gtx layout.Context, th *material.Theme) layout.Dimensions {
	return layout.Flex{
		Axis:    layout.Vertical,
		Spacing: layout.SpaceEvenly,
	}.Layout(gtx,
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return ce.layoutKernelField(gtx, th)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Spacer{Height: unit.Dp(20)}.Layout(gtx)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return ce.layoutControls(gtx, th)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Spacer{Height: unit.Dp(20)}.Layout(gtx)
		}),
	)
}

func (ce *ConvolutionEditor) layoutKernelField(gtx layout.Context, th *material.Theme) layout.Dimensions {
	return layout.Flex{
		Axis:    layout.Vertical,
		Spacing: layout.SpaceEvenly,
	}.Layout(gtx,
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return material.Label(th, unit.Sp(16), "Kernel Coefficients").Layout(gtx)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			border := widget.Border{
				Color: th.Palette.ContrastBg,
				Width: unit.Dp(2),
			}
			return border.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
				return material.Editor(th, &ce.KernelField, "Enter kernel values").Layout(gtx)
			})
		}),
	)
}

func (ce *ConvolutionEditor) layoutControls(gtx layout.Context, th *material.Theme) layout.Dimensions {
	return layout.Flex{
		Axis:    layout.Vertical,
		Spacing: layout.SpaceEvenly,
	}.Layout(gtx,
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Flex{
				Axis:    layout.Horizontal,
				Spacing: layout.SpaceEvenly,
			}.Layout(gtx,
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Label(th, unit.Sp(16), "Divisor:").Layout(gtx)
				}),
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Editor(th, &ce.DivisorField, "1.0").Layout(gtx)
				}),
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.CheckBox(th, &ce.AutoDivisor, "Auto").Layout(gtx)
				}),
			)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Spacer{Height: unit.Dp(10)}.Layout(gtx)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Flex{
				Axis:    layout.Horizontal,
				Spacing: layout.SpaceEvenly,
			}.Layout(gtx,
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Label(th, unit.Sp(16), "Offset:").Layout(gtx)
				}),
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Editor(th, &ce.OffsetField, "0.0").Layout(gtx)
				}),
			)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Spacer{Height: unit.Dp(10)}.Layout(gtx)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Flex{
				Axis:    layout.Horizontal,
				Spacing: layout.SpaceEvenly,
			}.Layout(gtx,
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Label(th, unit.Sp(16), "Anchor X:").Layout(gtx)
				}),
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Editor(th, &ce.AnchorXField, "1").Layout(gtx)
				}),
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Label(th, unit.Sp(16), "Anchor Y:").Layout(gtx)
				}),
				layout.Rigid(func(gtx layout.Context) layout.Dimensions {
					return material.Editor(th, &ce.AnchorYField, "1").Layout(gtx)
				}),
			)
		}),
	)
}

func (ce *ConvolutionEditor) UpdateKernel() string {
	kernelText := ce.KernelField.Text()
	rows := strings.Split(kernelText, "\n")
	ce.Kernel = make([][]float64, len(rows))
	if len(rows)%2 == 0 {
		return "Kernel size must be odd"
	}
	for i, row := range rows {
		cols := strings.Fields(row)
		if len(cols)%2 == 0 {
			return "Kernel size must be odd"
		}
		ce.Kernel[i] = make([]float64, len(cols))
		for j, col := range cols {
			value, err := strconv.ParseFloat(col, 64)
			if err != nil {
				return fmt.Sprintf("Invalid kernel value: %s", col)
			}
			ce.Kernel[i][j] = value
		}
	}
	if ce.AutoDivisor.Value {
		ce.Divisor = 0
		for i := 0; i < len(ce.Kernel); i++ {
			for j := 0; j < len(ce.Kernel[i]); j++ {
				ce.Divisor += ce.Kernel[i][j]
			}
		}
		if ce.Divisor == 0 {
			ce.Divisor = 1
		}
	} else {
		divisor, err := strconv.ParseFloat(ce.DivisorField.Text(), 64)
		if err != nil {
			return fmt.Sprintf("Invalid divisor value: %s", ce.DivisorField.Text())
		}
		ce.Divisor = divisor
	}
	offsetText := ce.OffsetField.Text()
	if offsetText == "" {
		offsetText = "0.0" // Default value if user did not input anything
	}
	offset, err := strconv.ParseFloat(offsetText, 64)
	if err != nil {
		return fmt.Sprintf("Invalid offset value: %s", offsetText)
	}
	ce.Offset = offset
	anchorX, err := strconv.Atoi(ce.AnchorXField.Text())
	if err != nil || anchorX < 0 || anchorX >= len(ce.Kernel[0]) {
		return fmt.Sprintf("Invalid anchor X value: %s", ce.AnchorXField.Text())
	}
	ce.AnchorX = anchorX
	anchorY, err := strconv.Atoi(ce.AnchorYField.Text())
	if err != nil || anchorY < 0 || anchorY >= len(ce.Kernel) {
		return fmt.Sprintf("Invalid anchor Y value: %s", ce.AnchorYField.Text())
	}
	ce.AnchorY = anchorY
	return ""
}

func (ce *ConvolutionEditor) LoadFilter(filter *ConvolutionFilter) {
	ce.Kernel = filter.Kernel
	ce.Divisor = filter.Divisor
	ce.Offset = filter.Offset
	ce.AnchorX = filter.AnchorX
	ce.AnchorY = filter.AnchorY

	var kernelText strings.Builder
	for i, row := range ce.Kernel {
		for _, value := range row {
			kernelText.WriteString(fmt.Sprintf("%f ", value))
		}
		if i == len(ce.Kernel)-1 {
			break
		}
		kernelText.WriteString("\n")
	}
	ce.KernelField.SetText(kernelText.String())
	ce.DivisorField.SetText(fmt.Sprintf("%f", ce.Divisor))
	ce.OffsetField.SetText(fmt.Sprintf("%f", ce.Offset))
	ce.AnchorXField.SetText(fmt.Sprintf("%d", ce.AnchorX))
	ce.AnchorYField.SetText(fmt.Sprintf("%d", ce.AnchorY))
}

func (ce *ConvolutionEditor) GetFilter() (*ConvolutionFilter, error) {
	err := ce.UpdateKernel()
	if err != "" {
		return nil, errors.New(err)
	}
	return &ConvolutionFilter{
		Kernel:  ce.Kernel,
		Divisor: ce.Divisor,
		Offset:  ce.Offset,
		AnchorX: ce.AnchorX,
		AnchorY: ce.AnchorY,
	}, nil
}
