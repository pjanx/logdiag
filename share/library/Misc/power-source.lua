-- Symbol name
local names =
{
	en = "Power source",
	cs = "Zdroj napětí"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-1, -2, 1, 2}

-- Terminals
local terminals = {{-1, 0}, {1, 0}}

-- Rendering
local render = function (cr)
	-- The vertical lines
	cr.move_to (-0.2, -1)
	cr.line_to (-0.2, 1)

	cr.move_to (0.2, -2)
	cr.line_to (0.2, 2)

	-- The contacts
	cr.move_to (-1, 0)
	cr.line_to (-0.2, 0)

	cr.move_to (0.2, 0)
	cr.line_to (1, 0)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("PowerSource", names, area, terminals, render)


