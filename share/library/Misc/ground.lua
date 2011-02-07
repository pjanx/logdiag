-- Symbol name
names =
{
	en = "Ground",
	cs = "Zem"
}

-- Render area in base units (X1, Y1, X2, Y2)
area = {-1, -1, 1, 2}

-- Terminals
terminals = {{0, -1}}

-- Rendering
render = function (cr)
	-- The vertical line
	cr.move_to (0, -1)
	cr.line_to (0, 0.5)

	-- The horizontal lines
	cr.move_to (-1, 0.5)
	cr.line_to (1, 0.5)

	cr.move_to (-0.75, 1.1)
	cr.line_to (0.75, 1.1)

	cr.move_to (-0.5, 1.7)
	cr.line_to (0.5, 1.7)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Ground", names, area, terminals, render)


