-- Symbol name
local names =
{
	en = "Terminal",
	cs = "Terminál",
	sk = "Terminál",
	pl = "Terminał"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-1, -0.5, 0.5, 0.5}

-- Terminal points
local terminals = {{-1, 0}}

-- Rendering
local render = function (cr)
	-- The circle
	cr.arc (0, 0, 0.3, 0, math.pi * 2)

	-- The contact
	cr.move_to (-1, 0)
	cr.line_to (-0.3, 0)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Terminal", names, area, terminals, render)


