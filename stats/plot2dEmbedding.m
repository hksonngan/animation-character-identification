function plot2dEmbedding(embedding, step, labels)

[rows cols] = size(embedding);

colors = zeros(rows, 3);

for i=1:step:rows
    color = [rand(1) rand(1) rand(1)];
    for j=i:i+step-1
        colors(j,:) = color;
    end
end

scatter(embedding(:,1), embedding(:,2), 10, colors, 'filled');

I = 1:step:rows;
p = findobj(gca,'Type','Patch');
legend(p(I), labels)

end

