-- Symbol name
local names =
{
	en = "Switch",
	cs = "Spínač",
	sk = "Spínač",
	pl = "Przełącznik",
	de = "Schalter"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1.5, 2, 0}

-- Terminal points
local terminals = {{-2, 0}, {2, 0}}

-- Rendering
local render = function (cr)
	-- The switch contact
	cr:move_to (1.3, -1.3)
	cr:line_to (-1, 0)

	-- The terminals
	cr:line_to (-2, 0)

	cr:move_to (1, 0)
	cr:line_to (2, 0)

	cr:stroke ()
end

-- Register the symbol
logdiag.register ("Switch", names, area, terminals, render)


